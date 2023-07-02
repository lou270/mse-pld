/******************************
| Project       : MSE Avionics
| Board         : SEQ
| Description   : Parameters
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/

// Project selection
#define MSE
// #define KRYPTONIT

// Auto includes
#if defined(MSE)
    #include "parameters_mse.h"
#elif defined(KRYPTONIT)
    #include "parameters_kryptonit.h"
#endif

