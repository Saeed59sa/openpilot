#!/usr/bin/env bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
UNDERLINE='\033[4m'
BOLD='\033[1m'
NC='\033[0m'

set -e

log_message() {
  echo -e "$(date '+%Y-%m-%d %H:%M:%S') - $1"
}

handle_error() {
  local exit_code=$?
  log_message "${RED}Error occurred at line $1. Exit code: $exit_code${NC}"

  if [[ $exit_code -eq 128 ]]; then
    log_message "${YELLOW}Attempting submodule recovery...${NC}"
    recover_submodules
  fi

  exit $exit_code
}

trap 'handle_error ${LINENO}' ERR

set_time_settings() {
  local timezone="$1"
  log_message "${GREEN}Setting timezone to $timezone${NC}"

  if ! sudo mount -o remount,rw / 2>/dev/null; then
    log_message "${YELLOW}Warning: Could not remount / as read-write${NC}"
  fi

  if ! sudo timedatectl set-timezone "$timezone" 2>/dev/null; then
    log_message "${YELLOW}Warning: Could not set timezone${NC}"
  fi

  if ! sudo timedatectl set-ntp true 2>/dev/null; then
    log_message "${YELLOW}Warning: Could not enable NTP${NC}"
  fi

  if ! sudo mount -o remount,ro / 2>/dev/null; then
    log_message "${YELLOW}Warning: Could not remount / as read-only${NC}"
  fi
}

check_network() {
  local max_attempts=3
  local attempt=1

  while [ $attempt -le $max_attempts ]; do
    log_message "Checking network connectivity (attempt $attempt/$max_attempts)..."

    if ping -c 3 -W 5 8.8.8.8 > /dev/null 2>&1; then
      log_message "${GREEN}Network connectivity confirmed${NC}"
      return 0
    fi

    if [ $attempt -lt $max_attempts ]; then
      log_message "${YELLOW}Network check failed, retrying in 3 seconds...${NC}"
      sleep 3
    fi

    ((attempt++))
  done

  log_message "${RED}Network connectivity failed after $max_attempts attempts${NC}"
  return 1
}

recover_submodules() {
  log_message "${YELLOW}Attempting to recover corrupted submodules...${NC}"

  local submodules=$(git config --file .gitmodules --get-regexp path | awk '{ print $2 }' || true)

  for submodule in $submodules; do
    if [ -d "$submodule" ]; then
      log_message "Cleaning submodule: $submodule"
      (cd "$submodule" && git clean -fd --exclude="__pycache__" --exclude="*.pyc" && git reset --hard HEAD) || {
        log_message "${YELLOW}Removing corrupted submodule: $submodule${NC}"
        rm -rf "$submodule"
      }
    fi
  done

  git submodule update --init --recursive --force || true
}

clean_git_repo() {
  log_message "${GREEN}Cleaning git repository (preserving __pycache__)...${NC}"

  git clean -fd --exclude="__pycache__" --exclude="*.pyc"

  git reset --hard HEAD
  git gc --auto
  git fsck --full || {
    log_message "${YELLOW}Git fsck found issues, attempting repair...${NC}"
    git fsck --full --unreachable --dangling || true
  }
}

needs_cleaning() {
  if ! git diff --quiet || ! git diff --cached --quiet; then
    return 0
  fi

  local untracked_files=$(git ls-files --others --exclude-standard --exclude="__pycache__" --exclude="*.pyc")
  if [ -n "$untracked_files" ]; then
    return 0
  fi

  return 1
}

configure_git() {
  log_message "${GREEN}Configuring git settings...${NC}"

  local current_fetch=$(git config --get remote.origin.fetch 2>/dev/null || echo "")
  local desired_fetch="+refs/heads/*:refs/remotes/origin/*"

  if [ "$current_fetch" != "$desired_fetch" ]; then
    git config remote.origin.fetch "$desired_fetch"
    log_message "Updated remote fetch configuration"
  fi

  local ssl_verify=$(git config --global http.sslVerify 2>/dev/null || echo "")
  if [ "$ssl_verify" != "false" ]; then
    git config --global http.sslVerify false
    log_message "Set http.sslVerify to false"
  fi

  local submodule_recurse=$(git config --global submodule.recurse 2>/dev/null || echo "")
  if [ "$submodule_recurse" != "true" ]; then
    git config --global submodule.recurse true
    log_message "Set submodule.recurse to true"
  fi

  git config --global http.postBuffer 524288000
  git config --global http.lowSpeedLimit 1000
  git config --global http.lowSpeedTime 300
}

safe_fetch_and_reset() {
  local branch="$1"
  local max_retries=3
  local retry=1
  local needs_repo_cleaning=false

  while [ $retry -le $max_retries ]; do
    log_message "${GREEN}Fetching changes (attempt $retry/$max_retries)...${NC}"

    if [ $retry -gt 1 ] && [ "$needs_repo_cleaning" = true ]; then
      log_message "${YELLOW}Cleaning repository before retry...${NC}"
      clean_git_repo
      needs_repo_cleaning=false
    fi

    if timeout 300 git fetch --all --prune; then
      log_message "${GREEN}Fetch completed successfully${NC}"
      break
    else
      local fetch_exit_code=$?
      needs_repo_cleaning=true

      if [ $fetch_exit_code -eq 124 ]; then
        log_message "${YELLOW}Fetch timed out after 300 seconds${NC}"
      else
        log_message "${YELLOW}Fetch failed with exit code: $fetch_exit_code${NC}"
      fi

      if [ $retry -eq $max_retries ]; then
        log_message "${RED}Fetch failed after $max_retries attempts${NC}"
        log_message "${YELLOW}Performing final repository cleanup...${NC}"
        clean_git_repo
        return 1
      fi
      log_message "${YELLOW}Fetch failed, retrying in 10 seconds...${NC}"
      sleep 10
      ((retry++))
    fi
  done

  if ! git show-ref --verify --quiet "refs/remotes/origin/$branch"; then
    log_message "${RED}Remote branch origin/$branch does not exist${NC}"
    return 1
  fi

  log_message "${GREEN}Resetting to origin/$branch...${NC}"
  git reset --hard "origin/$branch"
}

safe_submodule_update() {
  log_message "${GREEN}Syncing submodules...${NC}"
  git submodule sync --recursive

  log_message "${GREEN}Updating submodules...${NC}"
  local max_retries=3
  local retry=1

  while [ $retry -le $max_retries ]; do
    if timeout 300 git submodule update --init --recursive --force; then
      log_message "${GREEN}Submodules updated successfully${NC}"
      return 0
    else
      local submodule_exit_code=$?
      if [ $submodule_exit_code -eq 124 ]; then
        log_message "${YELLOW}Submodule update timed out after 300 seconds${NC}"
      else
        log_message "${YELLOW}Submodule update failed with exit code: $submodule_exit_code (attempt $retry/$max_retries)${NC}"
      fi

      if [ $retry -eq $max_retries ]; then
        log_message "${RED}Submodule update failed after $max_retries attempts${NC}"
        recover_submodules
        return 1
      fi
      sleep 10
      ((retry++))
    fi
  done
}

cleanup_gone_branches() {
  local gone_branches=$(git branch -vv | grep ': gone]' | awk '{print $1}' | tr -d '*' || true)

  if [ -n "$gone_branches" ]; then
    log_message "${GREEN}Cleaning up gone branches...${NC}"
    echo "$gone_branches" | while read -r branch; do
      if [ -n "$branch" ] && [ "$branch" != "$(git rev-parse --abbrev-ref HEAD)" ]; then
        git branch -D "$branch" || true
        log_message "Deleted gone branch: $branch"
      fi
    done
  fi
}

compare_commits() {
  local branch="$1"

  local remote_commit=$(git ls-remote origin "$branch" | awk '{print $1}' 2>/dev/null || echo "")
  local local_commit=$(git rev-parse HEAD 2>/dev/null || echo "")

  if [ -z "$remote_commit" ] || [ -z "$local_commit" ]; then
    log_message "${RED}Could not retrieve commit information${NC}"
    return 1
  fi

  local remote_time=$(date -d @"$(git show -s --format=%ct "origin/$branch" 2>/dev/null || echo "0")" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || echo "Unknown")
  local local_time=$(date -d @"$(git show -s --format=%ct HEAD 2>/dev/null || echo "0")" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || echo "Unknown")

  echo -e "  Remote Commit: [ ${GREEN}${BOLD} $remote_commit ${NC} ] - $remote_time"
  echo -e "   Local Commit: [ ${GREEN}${BOLD} $local_commit ${NC} ] - $local_time"

  if [ "$remote_commit" = "$local_commit" ]; then
    echo -e "\nCommit is ${GREEN}${BOLD}match${NC}. Proceeding with restart...\n"
    return 0
  else
    echo -e "\nCommit is ${RED}${BOLD}not match${NC}. Update may have failed.\n"
    return 1
  fi
}

main() {
  log_message "${GREEN}Starting gitpull script...${NC}"

  # Check if we're in the right directory
  if [ ! -d "/data/openpilot" ]; then
    log_message "${RED}/data/openpilot directory not found${NC}"
    exit 1
  fi

  cd /data/openpilot

  if [ -f "/data/openpilot/prebuilt" ]; then
    echo -n "0" > /data/params/d/PrebuiltEnable 2>/dev/null || true
    sudo rm -f prebuilt
    log_message "Removed prebuilt flag"
  fi

  if ! check_network; then
    touch /data/check_network
    log_message "${RED}Network check failed, exiting${NC}"
    exit 1
  fi

  local lang=$(cat /data/params/d/LanguageSetting 2>/dev/null || echo "")
  case "$lang" in
    "main_ko")
      set_time_settings "Asia/Seoul"
      ;;
    "main_en")
      set_time_settings "America/New_York"
      ;;
    *)
      log_message "${YELLOW}Unknown language setting: $lang${NC}"
      ;;
  esac

  local branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "master")
  log_message "Current branch: $branch"

  configure_git

  if needs_cleaning; then
    log_message "${YELLOW}Repository has uncommitted changes or untracked files, cleaning...${NC}"
    clean_git_repo
  else
    log_message "${GREEN}Repository is clean, skipping cleanup${NC}"
    git reset --hard HEAD
  fi

  if ! safe_fetch_and_reset "$branch"; then
    log_message "${RED}Failed to fetch and reset${NC}"
    exit 1
  fi

  if ! safe_submodule_update; then
    log_message "${YELLOW}Submodule update had issues, but continuing...${NC}"
  fi

  log_message "${GREEN}Git operations completed successfully!${NC}"

  cleanup_gone_branches

  if compare_commits "$branch"; then
    if [ -x "/data/openpilot/scripts/restart.sh" ]; then
      log_message "${GREEN}Executing restart script...${NC}"
      exec /data/openpilot/scripts/restart.sh
    else
      log_message "${RED}Restart script not found or not executable${NC}"
      exit 1
    fi
  else
    log_message "${YELLOW}Commits don't match, manual intervention may be required${NC}"
    exit 1
  fi
}

set +e
main "$@"
exit_code=$?

if [ $exit_code -ne 0 ]; then
  log_message "${RED}Script completed with errors (exit code: $exit_code)${NC}"
fi

exit $exit_code
