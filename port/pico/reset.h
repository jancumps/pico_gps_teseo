#ifndef RESET_H_
#define RESET_H_

#define RESET_PIN (18)
#define RESET_APPLY_MS (1)
// recover must be more than 3 seconds
#define RESET_RECOVER_MS (4000)

void reset();

#endif //RESET_H
