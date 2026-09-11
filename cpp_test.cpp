#include "implicit_net.h"

int main()
{
    in_state state = 0;
    return in_init(&state, 4, 0) == 1 ? 0 : 1;
}
