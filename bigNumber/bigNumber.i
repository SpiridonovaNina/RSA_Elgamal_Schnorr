%module bigNumber
%{
/* Includes the header in the wrapper code */
#include "bigNumber.h"
%}

/* Parse the header file to generate wrappers */
%include "bigNumber.h"
