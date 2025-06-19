#include "Setting.h"

Setting::Setting(const String& name, int steps)
  : _name(name), _steps(steps), _value(0), _active(false) {
  if (_steps < 2) _steps = 2;     // minimum steps
  if (_steps > 4096) _steps = 4096;
}

void Setting::activate() {
  _active = true;
}

void Setting::deactivate() {
  _active = false;
}

bool Setting::isActive() const {
  return _active;
}

void Setting::setValueFromPot(int raw) {
  // raw should be 0-1023 typically - no its 4095 >:(
  // int mapped = map(raw, 0, 4095, 0, _steps - 1);
  // int scaled = mapped * (4096 / _steps);
  // _value = constrain(scaled, 0, 4095);

  // Convert raw (0–4095) into step index (0–_steps-1) with rounding
  float stepSize = 4095.0f / (_steps - 1);
  int stepIndex = round(raw / stepSize);  // Step index from 0 to _steps - 1

  // Store scaled value back to full range for precise setting representation
  _value = constrain(round(stepIndex * stepSize), 0, 4095);
}

String Setting::getName() const {
  return _name;
}

// int Setting::getValue() const {
//   return _value;
// }

int Setting::getStepValue() const {
  float stepSize = 4095.0f / (_steps - 1);
  return round(_value / stepSize);
}

int Setting::getRawValue() const {
  return _value;
}

int Setting::getSteps() const {
  return _steps;
}
