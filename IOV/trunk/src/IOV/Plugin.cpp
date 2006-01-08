// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Plugin.h>


namespace inf
{

// NOTE: This is here in the .cpp file to ensure that inf::Plugin is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
Plugin::~Plugin()
{
   /* Do nothing. */ ;
}

// NOTE: This is here in the .cpp file to ensure that inf::Plugin is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
Plugin::Plugin()
   : mIsFocused(false)
{
   /* Do nothing. */ ;
}

}
