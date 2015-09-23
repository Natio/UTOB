//
//  Common.h
//  utob
//
//  Created by Paolo Coronati on 25/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#ifndef utob_Common_h
#define utob_Common_h
#include <stdio.h>

#define SERVER_BASE_PORT 38000

#undef DEBUG
#ifdef DEBUG
#define DLog(...) fprintf(stdout,__VA_ARGS__)
#else
#define DLog(...) /**/
#endif


#endif
