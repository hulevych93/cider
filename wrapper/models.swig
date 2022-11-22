%module example

%{
#include "models/aggregate_struct.h"
#include "models/free_functions.h"
%}

%include "models/free_functions.h"
%include "models/aggregate_struct.h"
