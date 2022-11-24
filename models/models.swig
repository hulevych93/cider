%module example

%{
#include "models/aggregate_struct.h"
#include "models/class_construct.h"
#include "models/free_functions.h"
#include "models/enum_test.h"
%}
%include "models/class_construct.h"
%include "models/free_functions.h"
%include "models/aggregate_struct.h"
%include "models/enum_test.h"
