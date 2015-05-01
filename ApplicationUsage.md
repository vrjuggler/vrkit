# vrkit Application Usage #

vrkit combines clustering features from VR Juggler and OpenSG. The result is a different application design that most VR Juggler programmers probably expect. Instead of writing a VR Juggler application that runs on all nodes of a cluster, users will most likely write plug-ins to `vrkit_app` and run one instance of it on the cluster master node while running the `slaveViewer` application on all other nodes. This is not the only way to use vrkit since both the master and the slave applications can be customized directly in conjunction with, or in lieu of, writing plug-ins. In any event, the fundamental concept is that the master application loads the scene graph once and performs all manipulations on it. OpenSG takes care of distributing the scene graph and all modifications made thereto for rendering on the slave nodes. The use of VR Juggler application-specific shared data in this design is largely--maybe even entirely--unnecessary.

## Configuration Files ##

vrkit uses Juggler configuration files (XML files ending with the extension `.jconf` that contain "config elements"), but it does not use the VR Juggler run-time reconfiguration fetaure. Instead, vrkit components are all configured statically when the application is initialized. This is a design limitation of vrkit since run-time reconfiguration is a very powerful and unique feature of VR Juggler. However, the decision was made very early on to keep the configuration of vrkit simple, and the use of run-time reconfiguration was considered unnecessary. That may change in the future.

Because run-time reconfiguration is not used, the vrkit config elements must be kept separate from the Juggler config elements. This means that `.jconf` files for vrkit components must be distinct from those for Juggler components. As we will see, this is handled by `vrkit_app` through the use of command line parameters.

## vrkit\_app ##

The standard vrkit "master" application is called `vrkit_app`, and its source can be found in the `src/Viewer` directory of the vrkit source tree. It accepts a number of command-line parameters. These are the following:

> `-j`/`--jconf`::
> > Provide the path to a VR Juggler configuration file. One or more files can be named by using multiple instances of this parameter. At least one file is required. This can be an absolute path, a relative path, or simply the name of a `.jconf` file that can be found using the configuration file search path. All config elements contained in the named file must be for Juggler components or other code units that use run-time reconfiguration. Those that are not will end up in the pending list.

> `-a`/`--app`::
> > Provide the path to a vrkit configuration file. One or more files can be named by using multiple instances of this parameter. At least one file is required. This can be an absolute or a relative path to a `.jconf` file. The configuration file search path is ''not'' used to find this file. All config elements contained in the named file must be for vrkit components. The handling of the config elemnts depends on what plug-ins are loaded, but in general, each component will look for one element matching its desired type. For example, if a vrkit configuration contains multiple `vrkit_viewer` config elements, only one will be used.

> `-d`/`--defs`::
> > Extend the config definition file search path to include a named directory. One or more directories containing config definition files (XML files named with the extension `.jdef`) can be specified by using multiple instances of this parameter. By default, `$VRKIT_BASE_DIR/share/vrkit/definitions` is added to the config definition search path if no instance of this parameter is passed on the command line. This is an alternative to using the `$JCCL_DEFINITION_PATH` environment variable, but `$JCCL_DEFINITION_PATH` will always be used if it is set.

> `-f`/`--file`::
> > Name a model file for OpenSG to load. For versions up to and including 0.50.0, the default value for this argument is `data/scenes/test_scene.osb`, a path that is relative to the root directory of the vrkit source tree. In general, it is probably better to use the Model Loader Plug-in to load models than this option, of which only one instance is allowed on the command line. With vrkit 0.50.1 and newer, no model is loaded by default, so users must either name the model they want to load using this option or use the Model Loader Plug-in.

If VR Juggler 2.3 or newer is being used, the additional option `--vrjmaster` for identifying the application as being on the master cluster node is also available.

A typical way of using `vrkit_app` is the following, executed from the root directory of the vrkit source tree:

```
% build.linux/instlinks/bin/vrkit_app -a src/Viewer/viewer-nav.jconf
                                      -j standalone.jconf
```

The above command will load the default model `data/scenes/test_scene.osb` with a vrkit configuration that supports mode switching, navigation, grabbing, and video capture (as of this writing). The VR Juggler configuration `standalone.jconf` is used and found relative to the configuration file search path.

If `vrkit_app` is being run in a cluster configuration, a common usage would be similar to the following:

```
% build.linux/instlinks/bin/vrkit_app -a src/Viewer/viewer-nav.jconf
                                      -a src/Viewer/viewer-cluster.mixin.jconf
                                      -j /path/to/site-cluster.jconf
```

In this example, we have added vrkit cluster configuration information in the "mix-in" file `src/Viewer/viewer-cluster.mixin.jconf`. The critical piece of information for vrkit's cluster configuration is the number of slave nodes that will be connecting.

The VR Juggler 2.3 form of the above example is the following:

```
% build.linux/instlinks/bin/vrkit_app -a src/Viewer/viewer-nav.jconf
                                      -a src/Viewer/viewer-cluster.mixin.jconf
                                      -j /path/to/site-cluster.jconf
                                      --vrjmaster
```

## slaveViewer ##

When running in a cluster configuration, the vrkit `slaveViewer` application, or some other custom slave application will be run. The job of this application is generally quite simple: render the scene graph that OpenSG provides via a network connection to the master application. Custom rendering may occur, but that requires a custom slave application. Otherwise, VR Juggler input data sharing will keep the rendering synchronized and the view frustum up to date.

The standard vrkit `slaveViewer` application, the source for which is found in the `src/SlaveViewer` directory of the vrkit source tree, accepts several command line arugments. They are the following:


> `-j`/`--jconf`::
> > Provide the path to a VR Juggler configuration file. One or more files can be named by using multiple instances of this parameter. At least one file is required when using a version of VR Juggler older than 2.3.0. For VR Juggler 2.3 and beyond, this option is not available. The parameter value can be an absolute path, a relative path, or simply the name of a `.jconf` file that can be found using the configuration file search path. All config elements contained in the named file must be for Juggler components or other code units that use run-time reconfiguration. Those that are not will end up in the pending list. For versions of VR Juggler prior to 2.3, this will almost always be the same file(s) that are passed to the master application.

> `-a`/`--addr`::
> > Give the IP address or host name and port number for the master application in the form `address:port`. This option is ''required''. The values used here depend on how the master application cluster component is configured (see above). Do not confuse this option with the `-a`/`--app` option used by `vrkit_app`. For the slave application, we provide ''no'' vrkit config elements whatsoever.

> `-r`/`--root`::
> > Name the root node of the scene graph being shared by the master. Again, the name of this depends on how the master application cluster component is configured, but this will most likely have the value `"RootNode"`, which happens to be the default if this parameter is not given on the command line.

> `-m`/`--mask`::
> > The value of the render action traversal mask in base-8, base-10, or base-16 notation. This is only required when the master application is using a custom traversal mask. In that event, the slave viewer must use the same traversal mask in order to get proper rendering behavior. The default value is `0xfffffffff`, the same as the `vrkit::Viewer` class default.

If VR Juggler 2.3 or newer is being used, the additional option `--vrjslave` for identifying the application as being on the slave cluster node must be used.

An example of using this application with VR Juggler 2.2 (or 2.0) is the following:

```
slaveViewer -j /path/to/site-cluster.jconf -a master:34000
```

This assumes that the name of the master cluster node is "master" and that it is listening on port 34000 for incoming vrkit slave application connections.

The VR Juggler 2.3 form of the above example is as follows:

```
slaveViewer --vrjslave -a master:34000
```