# vrkit Development #

## Design Philosophy ##

vrkit sits above VR Juggler and OpenSG and aims to combine the two in powerful, yet easy-to-use ways. The core vrkit library is comprised of utility code, and in no small way, it is a collection of reusable classes and function that represent common aspects of VR Juggler application construction. For example, `vrkit::util::DigitalCommand` captures digital button input, something seen in nearly all VR Juggler applications, in a manner that is far more flexible than what is normally demonstrated in VR Juggler sample applications and documentation.

As for packaged immersive application functionality, users will turn their attention to plug-ins, all of which rely upon common code in the vrkit core library. The idea with the plug-ins is to capture reusable features that are ready to use in any and all vrkit-based applications. Indeed, one could easily use `vrkit_app` with a wide array of plug-ins to get a feature-rich immersive application without writing any C++ code. Thus, we strive to keep functionality out of the application core and capture it instead in plug-ins.