/*
 * Copyright (C) 2008-2014 Ole André Vadla Ravnås <ole.andre.ravnas@tillitech.com>
 *
 * Licence: wxWindows Library Licence, Version 3.1
 */

#ifndef __GUM_H__
#define __GUM_H__

#include <gum/gumdefs.h>

#include <gum/gumbacktracer.h>
#include <gum/gumevent.h>
#include <gum/gumeventsink.h>
#include <gum/guminterceptor.h>
#include <gum/guminvocationlistener.h>
#include <gum/gumlist.h>
#include <gum/gummemory.h>
#include <gum/gummemoryaccessmonitor.h>
#include <gum/gumprocess.h>
#include <gum/gumreturnaddress.h>
#include <gum/gumscript.h>
#include <gum/gumstalker.h>
#include <gum/gumsymbolutil.h>

G_BEGIN_DECLS

typedef guint GumFeatureFlags;

enum _GumFeatureFlags
{
  GUM_FEATURE_SYMBOL_LOOKUP = (1 << 0),

  GUM_FEATURE_NONE          = 0,
  GUM_FEATURE_ALL           = (GUM_FEATURE_SYMBOL_LOOKUP),
  GUM_FEATURE_DEFAULT       = GUM_FEATURE_ALL
};

GUM_API void gum_init (void);
GUM_API void gum_init_with_features (GumFeatureFlags features);
GUM_API void gum_deinit (void);

G_END_DECLS

#endif
