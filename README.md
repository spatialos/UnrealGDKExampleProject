# SpatialOS Unreal GDK Example Project

* **LICENSE:** Use of the contents of this repository is subject to the [license](LICENSE.md)
* **REPOSITORY:** [github.com/improbable/UnrealGDKExampleProject](https://github.com/spatialos/UnrealGDKThirdPersonShooter)


This project is an example shooter game which uses the [SpatialOS GDK for Unreal](https://github.com/spatialos/unrealGDK). 

It is both a reference project and a starting point for those building games on SpatialOS and Unreal Engine 4.


For setup instructions, follow the [Get started](https://docs.improbable.io/unreal/latest/get-started/introduction) guide in the GDK for Unreal documentation. This guide uses the [GDK for Unreal Starter Project](https://github.com/spatialos/UnrealGDKStarterProject) as an example. When you follow the installation instructions, you need to replace the "Starter Project" repository with the Unreal GDK Example Project one.

For more information, see the [SpatialOS GDK for Unreal documentation](https://docs.improbable.io/unreal/latest/).

####  *WIP NOTES*
When launching a deployment, for the deployment manager to work, the dev auth token must be updated. (line 119 of `DeploymentsPlayerController.cpp`, resembling `PITParams->development_authentication_token_id = "TODO"`. See [Docs](https://docs.improbable.io/reference/13.6/shared/spatial-cli/spatial-project-auth-dev-auth-token-create) for how to create/update the dev auth token.)
You can check your token is sufficiently up-to-date by running the `Deployments` map in-editor, with `Spatial Networking` set to true, and `Run dedicated server` set to false. If there is the option to `Quick Join`, it's up to date/correctly authed.

####  Helper scripts

This repository contains a set of helper scripts to get you started.

| Helper script | Description |
| --- | --- |
| `LaunchSpatial.bat` | Starts a local SpatialOS deployment with the default launch configuration. |
| `LaunchServer.bat` | Starts an Unreal server-worker, and connects it to the local deployment. |
| `LaunchClient.bat` | Starts an Unreal client-worker, and connects it to the local deployment. |
| `ProjectPaths.bat` | Used by the `LaunchClient.bat`, `LaunchServer.bat` and `LaunchSpatial.bat` to specify the project environment when those scripts are run |


#### Give us feedback
We have released the GDK for Unreal this early in development because we want your feedback. Please come and talk to us about the software and the documentation via: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) - [GitHub issues in this repository](https://github.com/spatialos/UnrealGDK/issues).

#### Where to get the GDK and related projects
The GDK and its example projects are available on GitHub.
* [GDK: github.com/spatialos/UnrealGDK](https://github.com/spatialos/UnrealGDK)
* [The SpatialOS Unreal Engine](https://github.com/improbableio/UnrealEngine/tree/4.20-SpatialOSUnrealGDK)
**NOTE:** This link may give you a 404.
* [Starter Project](https://github.com/spatialos/UnrealGDKStarterProject)
* [Third-Person Shooter Game](https://github.com/spatialos/UnrealGDKThirdPersonShooter) (Not actively developed)
* [The Test Suite](https://github.com/spatialos/UnrealGDKTestSuite)

#### Public contributors
We are not currently accepting public contributions. However, we are accepting [issues](https://github.com/spatialos/UnrealGDK/issues) and we do want your feedback.

&copy; 2018 Improbable