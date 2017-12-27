#pragma once

#define GPIO(p,pin,on) if (on) { PORT ## p |= _BV(P ## p ## pin); } else { PORT ## p &=~ _BV(P ## p ## pin); }


