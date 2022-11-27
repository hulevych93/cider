%module example

%{
#include "models/polymorphic_types.h"
#include "models/aggregate_struct.h"
#include "models/derived_agregate_types.h"
#include "models/derived_class_types.h"
#include "models/some_final_class.h"
#include "models/free_functions.h"
#include "models/enum_test.h"
%}
%include "models/some_final_class.h"
%include "models/free_functions.h"
%include "models/aggregate_struct.h"
%include "models/enum_test.h"
%include "models/polymorphic_types.h"
%include "models/derived_agregate_types.h"
%include "models/derived_class_types.h"
