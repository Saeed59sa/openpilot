from openpilot.common.params_pyx import Params, ParamKeyType, UnknownKeyName
assert Params
assert ParamKeyType
assert UnknownKeyName

# Default values for HybridTACC parameters
HYBRID_TACC_DEFAULTS = {
  "HybridTACCEnabled": "0",
  "HybridTACCMode": "Auto",
  "HybridTACCSmoothness": "0.5",
  "HybridTACCSwitchDelay": "1.0",
  "HybridTACCLearnerEnabled": "0",
}

if __name__ == "__main__":
  import sys

  params = Params()
  key = sys.argv[1]
  assert params.check_key(key), f"unknown param: {key}"

  if len(sys.argv) == 3:
    val = sys.argv[2]
    print(f"SET: {key} = {val}")
    params.put(key, val)
  elif len(sys.argv) == 2:
    print(f"GET: {key} = {params.get(key)}")
