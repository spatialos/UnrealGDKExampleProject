* Runtime Version - 0.4.3
* UnrealEngine Fork - https://github.com/improbableio/UnrealEngine/tree/4.24-SpatialOSUnrealGDK-Blast420
* UnrealGDK - on master, commit - 9b9606775b4d8933fea205a1bd23ab374503fd4e
* GameWorks - https://github.com/yunjielu/GameWorks/tree/Blast420_GDK424, cd to folder "Engine/Plugins", git clone GameWorks into Plugins


Build this example project, enter the scene, you can press "0" to generate BlastCubes at runtime

Press the "0" once - generate 1950(65x30) cubes
Press the "0" twice - generate another 1950(65x30) cubes
Press the "0" three times - destroy all cubes

You can quickly press "0" twice to generate 3900 cubes and get three result:
* Successfully replicated all cubes to runtime but takes 40 seconds to finish the process
* Runtime will disconnect server worker
* Always retrying to send RPC
