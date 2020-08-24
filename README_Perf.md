* Runtime Version - 0.4.3
* UnrealEngine Fork - https://github.com/improbableio/UnrealEngine/tree/4.24-SpatialOSUnrealGDK-Blast420
* UnrealGDK - on master, commit - 9b9606775b4d8933fea205a1bd23ab374503fd4e
* GameWorks - https://github.com/yunjielu/GameWorks/tree/Blast420_GDK424, cd to folder "Engine/Plugins", git clone GameWorks into Plugins

Build this example project, enter the scene, you can press "0" to generate BlastCubes at runtime

Press the "0" once - generate 1950(65x30) cubes

Press the "0" twice - generate another 1950(65x30) cubes

Press the "0" three times - destroy all cubes

You can quickly press "0" twice to generate 3900 cubes and get three result:
* Turn off ringbuffer, successfully replicated all cubes to runtime but takes 40 seconds to finish the process

* Turn off ringbuffer, runtime will disconnect server worker
  error message - "Disconnected: the server shutdoown with message: TRANSPORT: non-blocking attempt to send message to client with no available space in receive window, terminating, Driver = GameNetDriver SpatialNetDriver_2"
  
* Turn off ringbuffer, always retrying to send RPC and 
  warning message - "LogSpatialSender: Sending create entity request for BlastCubeBlueprint_C_7082 with EntityId 7814, HasAuthority: 1
LogSpatialActorChannel: Warning: Create entity request timed out. Retrying. Actor BlastCubeBlueprint_C_7051, request id: 15829, entity id: 0, message: timed out locally"

* Turn on ringbuffer, ringbuffer would be overflowed and drop messages
  
