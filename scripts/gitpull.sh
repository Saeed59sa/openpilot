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

if [ -d "/data/openpilot" ]; then
  log_message "${GREEN}Changing directory to /data/openpilot...${NC}"
  pushd /data/openpilot
else
  log_message "${YELLOW}/data/openpilot directory not found. Assuming current directory is the target.${NC}"
fi

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
  log_message "Checking network connectivity..."
  local dns_servers=("8.8.8.8" "8.8.4.4" "1.1.1.1" "1.0.0.1")

  for dns in "${dns_servers[@]}"; do
    if ping -c 3 -W 5 "$dns" > /dev/null 2>&1; then
      log_message "${GREEN}Network connectivity confirmed via $dns${NC}"
      return 0
    else
      log_message "${YELLOW}Failed to reach $dns${NC}"
    fi
  done

  log_message "${RED}All network connectivity tests failed${NC}"
  return 1
}

recover_submodules() {
  log_message "${YELLOW}Attempting to recover corrupted submodules...${NC}"

  local submodules=$(git config --file .gitmodules --get-regexp path | awk '{ print $2 }' || true)

  for submodule in $submodules; do
    if [ -d "$submodule" ]; then
      log_message "Cleaning submodule: $submodule"
      (cd "$submodule" && git reset --hard HEAD) || {
        log_message "${YELLOW}Removing corrupted submodule: $submodule${NC}"
        rm -rf "$submodule"
      }
    fi
  done

  git submodule update --init --recursive --force || true
}

clean_git_repo() {
  log_message "${GREEN}Cleaning git repository (preserving __pycache__)...${NC}"
  git clean -fd
  git gc --auto
  git fsck --full || {
    log_message "${YELLOW}Git fsck found issues, attempting repair...${NC}"
    git fsck --full --unreachable --dangling || true
  }
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

  git config --global http.postBuffer 1048576000
  git config --global http.lowSpeedLimit 1000
  git config --global http.lowSpeedTime 300
  git config --global core.preloadindex true
  git config --global transfer.bundleURI true
  git config --global fetch.parallel 8
  git config --global submodule.fetchJobs 8
  git config --global diff.ignoreSubmodules untracked
}

safe_fetch_and_reset() {
  local branch="$1"
  local max_retries=2
  local retry=1

  while [ $retry -le $max_retries ]; do
    log_message "${GREEN}Fetching changes (attempt $retry/$max_retries)...${NC}"

    if [ $retry -eq 1 ]; then
      local last_fetch_time=$(stat -c %Y .git/FETCH_HEAD 2>/dev/null || echo "0")
      local current_time=$(date +%s)
      local time_diff=$((current_time - last_fetch_time))

      if [ $time_diff -lt 3600 ]; then
        log_message "${GREEN}Attempting incremental fetch (last fetch was ${time_diff}s ago)...${NC}"
        if timeout 300 git fetch origin "$branch" --depth=10; then
          log_message "${GREEN}Incremental fetch completed successfully${NC}"
          break
        else
          log_message "${YELLOW}Incremental fetch failed, trying full fetch...${NC}"
        fi
      fi
    fi

    log_message "${GREEN}Performing full fetch with progress...${NC}"
    if timeout 600 git fetch --all --prune --progress; then
      log_message "${GREEN}Full fetch completed successfully${NC}"
      break
    else
      local fetch_exit_code=$?
      if [ $fetch_exit_code -eq 124 ]; then
        log_message "${YELLOW}Fetch timed out after 600 seconds${NC}"
      else
        log_message "${YELLOW}Fetch failed with exit code: $fetch_exit_code${NC}"
      fi

      if [ $retry -eq $max_retries ]; then
        log_message "${RED}Fetch failed after $max_retries attempts${NC}"
        clean_git_repo
        return 1
      fi
      log_message "${YELLOW}Fetch failed, retrying in 15 seconds...${NC}"
      sleep 15
      ((retry++))
    fi
  done

  if ! git show-ref --verify --quiet "refs/remotes/origin/$branch"; then
    log_message "${RED}Remote branch origin/$branch does not exist${NC}"
    return 1
  fi

  if ! git diff --quiet || ! git diff --cached --quiet; then
    log_message "${YELLOW}Discarding local changes...${NC}"
    git reset --hard HEAD
  fi

  log_message "${GREEN}Resetting to origin/$branch...${NC}"
  git reset --hard "origin/$branch"

  return 0
}

safe_submodule_update() {
  log_message "${GREEN}Syncing submodules...${NC}"
  git submodule sync --recursive

  log_message "${GREEN}Updating submodules...${NC}"
  local max_retries=2
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

  local remote_commit=$(git ls-remote origin "$branch" | awk '{print substr($1,1,7)}' 2>/dev/null || echo "")
  local local_commit=$(git rev-parse --short=7 HEAD 2>/dev/null || echo "")

  if [ -z "$remote_commit" ] || [ -z "$local_commit" ]; then
    log_message "${RED}Could not retrieve commit information${NC}"
    return 1
  fi

  local remote_time=$(date -d @"$(git show -s --format=%ct "origin/$branch" 2>/dev/null || echo "0")" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || echo "Unknown")
  local local_time=$(date -d @"$(git show -s --format=%ct HEAD 2>/dev/null || echo "0")" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || echo "Unknown")

  echo -e "\n Local Commit: ($local_time) [ ${GREEN}${BOLD}$local_commit${NC} ]"
  echo -e "Remote Commit: ($remote_time) [ ${GREEN}${BOLD}$remote_commit${NC} ]"

  # Return 0 for match, 1 for not match
  [ "$local_commit" == "$remote_commit" ]
}

main() {
  log_message "${GREEN}Starting Git pull script...${NC}"

  if [ -f "/data/openpilot/prebuilt" ]; then
    echo -n "0" > /data/params/d/PrebuiltEnable 2>/dev/null || true
    sudo rm -f prebuilt
    log_message "Removed prebuilt"
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

  if ! safe_fetch_and_reset "$branch"; then
    log_message "${RED}Failed to fetch and reset${NC}"
    exit 1
  fi

  if ! safe_submodule_update; then
    log_message "${YELLOW}Submodule update had issues, but continuing...${NC}"
  fi

  log_message "${GREEN}Git pull process completed${NC}"

  cleanup_gone_branches

  if compare_commits "$branch"; then
    echo -e "\nCommit Compare [${GREEN}${BOLD} match ${NC}]\n"
    if [ -x "/data/openpilot/scripts/restart.sh" ]; then
      log_message "${GREEN}Executing restart script...\n${NC}"
      exec /data/openpilot/scripts/restart.sh
    else
      log_message "${RED}Restart script not found, exiting\n${NC}"
      exit 1
    fi
  else
    echo -e "\nCommit Compare [${RED}${BOLD} not match ${NC}]\n"
    log_message "${RED}Git pull process failed: Local and remote commits do not match after reset.${NC}"
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
