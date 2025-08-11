# SDRaceMode package initializer
# Test guard: safe fallback if native Params missing
try:
  from openpilot.common.params import Params as _RealParams  # noqa
  Params = _RealParams  # type: ignore
  NATIVE_OK = True
except Exception:
  NATIVE_OK = False
  class Params:  # dummy fallback for tests
    _store = {}
    def get(self, k):
      b = k.encode() if isinstance(k, str) else k
      return self._store.get(b)
    def put(self, k, v):
      b = k.encode() if isinstance(k, str) else k
      self._store[b] = v if isinstance(v, (bytes, bytearray)) else str(v).encode()
