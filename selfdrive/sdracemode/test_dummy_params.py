import importlib
import sys
import types


def test_fallback_params(monkeypatch):
  dummy = types.ModuleType("params")  # no Params attribute
  monkeypatch.setitem(sys.modules, "openpilot.common.params", dummy)
  sys.modules.pop("selfdrive.sdracemode", None)
  sdr = importlib.import_module("selfdrive.sdracemode")
  assert sdr.NATIVE_OK is False
  p = sdr.Params()
  p.put("x", "y")
  assert p.get("x") == b"y"
