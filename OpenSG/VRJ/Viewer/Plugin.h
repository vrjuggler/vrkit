#ifndef PLUGIN_H
#define PLUGIN_H

class inf::Grabber;

namespace inf
{
/**
 * A plugin is an abstract interface that allows for an extension point in the system.  It could be used to add capabilities to the system.
 *
 * There may need to be "controller" plugins and "system" plugins.  This is still a little sketchy to me.
 *
 * NOTE: Still be fleshed out
 */
class Plugin
{
private:
   Grabber * lnkGrabber;
};

}

#endif