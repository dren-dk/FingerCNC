#pragma once

#include <avr/pgmspace.h>
#include "eventnames.h"

void addEvent(Event event);
Event takeEvent();

PGM_P getEventName(Event event);
