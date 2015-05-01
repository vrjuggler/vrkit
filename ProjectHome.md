# vrkit: This One Goes to 11 #

vrkit combines [VR Juggler](http://www.vrjuggler.org/) and [OpenSG](http://www.opensg.org/) to facilitate the rapid creation feature-rich immersive applications for Linux, Mac OS X, and Microsoft Windows. Features are deployed through plug-ins that are combined through one or more configuration files at run time. This allows for easy reuse of highly flexible code rather than duplication of code/features from one application to another.

vrkit currently comes with 26 plug-ins encompassing features such as navigation, object grabbing, logo display, model switching, and video capture. The standard application known as "vrkit\_app" can load and utilize any vrkit plug-in, thus allowing developers to focus on creating self-contained, reusable features for immersive software.

## Clustered from Day One ##

Applications based on vrkit leverage the clustering technologies of both VR Juggler and OpenSG. VR Juggler is used for device input sharing and node synchronization, and OpenSG is used for scene graph distribution. In general, there will be one copy of the "master" immersive application running that loads and distributes the scene graph over the cluster network. All the other nodes of the cluster run a simple "slave" application whose sole purpose is to receive the scene graph from OpenSG and render it.

## Designed to be (Re)used ##

vrkit is more than just a collection of plug-ins. Its core library contains a wide selection of utility code that dramatically simplifies the process of writing immersive applications based on VR Juggler. Common tasks are captured in components that are designed to be reused and extended. Modern C++ design guidelines are employed to leverage proven concepts and patterns.

## Open Source Virtual Reality ##

Continuing the tradition of VR Juggler and OpenSG, vrkit is open source software licensed under the [GNU LGPL](http://www.gnu.org/licenses/lgpl.html). The source is freely available to be used by commercial and non-commercial users. Contributions, bug fixes, and new features are always welcome. If you are interested in becoming a vrkit developer, subscribe to the vrkit-devel mailing list and introduce yourself to get the ball rolling.