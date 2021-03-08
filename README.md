yunjie:
SoufaMultiWorkerSettings
ZoningGridBasedLBStrategy


# SpatialOS GDK for Unreal Example Project

<img src="https://user-images.githubusercontent.com/2433131/58086122-f28af200-7bb5-11e9-9b96-b13b747130d0.png" height="70%" width="70%">

- **LICENSE:** Use of the contents of this repository is subject to the [license](LICENSE.md)

The SpatialOS Game Development Kit (GDK) for Unreal is an Unreal Engine fork and plugin with associated projects. It gives you the features of [SpatialOS](https://documentation.improbable.io/spatialos-overview/docs), within the familiar workflows and APIs of Unreal Engine. For more information, see the GDK's [documentation website](https://documentation.improbable.io/gdk-for-unreal/docs).

> To understand the feature-completeness, stability, performance, and support levels you can expect from the GDK, see the [product maturity lifecycle page](https://documentation.improbable.io/gdk-for-unreal/docs/product-maturity-lifecycle). For more information, visit the [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support](https://documentation.improbable.io/gdk-for-unreal/docs/unreal-features-support) pages, and contact us via our forums, or on Discord.

This is the repository for the Example Project, an example shooter game that uses the GDK.

In addition to the Example Project, the GDK also contains:
 
* [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine)

    You must be a member of the [Epic Games organization](https://github.com/EpicGames) on GitHub to access this. If you aren't, the link returns a 404 error.
* [The GDK plugin](https://github.com/spatialos/UnrealGDK)

## About the Example Project 
The Example Project contains gameplay and assets that are representative of a basic first-person shooter running on SpatialOS. If you want to make your own project from scratch, use the Starter Template by following the [Starter Template guide](https://documentation.improbable.io/gdk-for-unreal/docs/sample-projects-starter-template-introduction) in the GDK for Unreal documentation.

For setup instructions, follow the [Example Project guide](https://documentation.improbable.io/gdk-for-unreal/docs/sample-projects-example-project-introduction) in the GDK for Unreal documentation. 

For more information, see the [SpatialOS GDK for Unreal documentation](https://documentation.improbable.io/gdk-for-unreal/docs).

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

#### Public contributors

We are not currently accepting public contributions. However, we are accepting [issues](https://github.com/spatialos/UnrealGDK/issues) and we do want your feedback.

&copy; 2020 Improbable
