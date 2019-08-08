# SpatialOS GDK for Unreal Example Project

<img src="https://user-images.githubusercontent.com/2433131/58086122-f28af200-7bb5-11e9-9b96-b13b747130d0.png" height="70%" width="70%">

- **LICENSE:** Use of the contents of this repository is subject to the [license](LICENSE.md)

The SpatialOS Game Development Kit (GDK) for Unreal is an Unreal Engine fork and plugin with associated projects. It gives you the features of [SpatialOS](https://spatialos.improbable.io/docs/reference/latest), within the familiar workflows and APIs of Unreal Engine. For more information, please see the GDK's [documentation website](https://docs.improbable.io/unreal/latest).

> The SpatialOS GDK for Unreal is in alpha. It is ready to use for development of single-server games, but not recommended for public releases. We are committed to rapid development of the GDK to provide a performant release - for information on this, see our [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support](https://docs.improbable.io/unreal/latest/unreal-features-support) pages, and contact us via our forums, or on Discord.

This is the repository for the Example Project, an example shooter game that uses the GDK.

In addition to the Example Project, the GDK also contains:
 
* [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine)
* [The GDK plugin](https://github.com/spatialos/UnrealGDK)

## About the Example Project 
The Example Project contains gameplay and assets that are representative of a basic first-person shooter running on SpatialOS. If you want to make your own project from scratch, use the [Starter Template](https://docs.improbable.io/unreal/alpha/content/get-started/gdk-template).

For setup instructions, follow the [Example Project guide](https://docs.improbable.io/unreal/latest/content/get-started/example-project/exampleproject-intro)  in the GDK for Unreal documentation. 

For more information, see the [SpatialOS GDK for Unreal documentation](https://docs.improbable.io/unreal/latest/).

#### Game controls

The following controls are bound in-game, for gameplay and testing.

| Key Binding       | Function          |
| ----------------- | ----------------- |
| W,A,S,D           | Standard movement |
| Space             | Jump              |
| Shift             | Sprint            |
| Left Click        | Fire              |
| Number keys 1 - 0 | Select weapon     |

#### Helper scripts

This repository contains a set of helper scripts to get you started.

| Helper script       | Description                                                  |
| ------------------- | ------------------------------------------------------------ |
| `LaunchSpatial.bat` | Starts a local SpatialOS deployment with the default launch configuration. |
| `LaunchServer.bat`  | Starts an Unreal server-worker, and connects it to the local deployment. |
| `LaunchClient.bat`  | Starts an Unreal client-worker, and connects it to the local deployment. |
| `ProjectPaths.bat`  | Used by the `LaunchClient.bat`, `LaunchServer.bat` and `LaunchSpatial.bat` to specify the project environment when those scripts are run |

#### Give us feedback

We have released the GDK for Unreal this early in development because we want your feedback. Please come and talk to us about the software and the documentation via: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) - [GitHub issues in this repository](https://github.com/spatialos/UnrealGDK/issues).

#### Where to get the GDK and related projects

The GDK and related projects are available on GitHub.

- [GDK: github.com/spatialos/UnrealGDK](https://github.com/spatialos/UnrealGDK)
- [The SpatialOS Unreal Engine](https://github.com/improbableio/UnrealEngine/tree/4.20-SpatialOSUnrealGDK)
  **NOTE:** This link may give you a 404. See the [Unreal GDK documentation](https://docs.improbable.io/unreal/alpha/content/get-started/build-unreal-fork) for more information.
- [Third-Person Shooter](https://github.com/spatialos/UnrealGDKThirdPersonShooter) (Not actively developed)
- [The Test Suite](https://github.com/spatialos/UnrealGDKTestSuite)

#### Public contributors

We are not currently accepting public contributions. However, we are accepting [issues](https://github.com/spatialos/UnrealGDK/issues) and we do want your feedback.

&copy; 2019 Improbable
