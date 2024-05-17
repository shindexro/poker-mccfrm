#include "enums/action.h"

namespace poker {
std::ostream &operator<<(std::ostream &out, const Action &value) {
  std::string s = [value] {
#define PROCESS_VAL(p)                                                         \
  case (p):                                                                    \
    return #p;
    switch (value) {
      PROCESS_VAL(None);
      PROCESS_VAL(Fold);
      PROCESS_VAL(Call);
      PROCESS_VAL(Raise);
      PROCESS_VAL(Raise1);
      PROCESS_VAL(Raise2);
      PROCESS_VAL(Raise3);
      PROCESS_VAL(Raise4);
      PROCESS_VAL(Raise5);
      PROCESS_VAL(Raise6);
      PROCESS_VAL(Raise7);
      PROCESS_VAL(Raise8);
      PROCESS_VAL(Raise9);
      PROCESS_VAL(Allin);
    }
    return "N/A";
#undef PROCESS_VAL
  }();
  return out << s;
}
} // namespace poker
