#ifndef INPUT_COMPONENT_H
#define INPUT_COMPONENT_H

#include "primal.h"

using namespace primal;

DEFINE_COMPONENT(InputTestComponent, Component, false)
private:
uint64_t handleA, handleB, handleC;

public:
void onEnable() override;
void onDisable() override;
void update() override;

DEFINE_COMPONENT_END(InputTestComponent, Component)

#endif
