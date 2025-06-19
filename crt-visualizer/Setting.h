#ifndef SETTING_H
#define SETTING_H

#include <Arduino.h>

class Setting {
  public:
    Setting(const String& name, int steps = 4096);
    
    void activate();
    void deactivate();
    bool isActive() const;

    void setValueFromPot(int raw);

    String getName() const;
    // int getValue() const;
    int getStepValue() const;
    int getRawValue() const;
    int getSteps() const;

  private:
    String _name;
    int _steps;
    int _value;
    bool _active;
};

#endif
