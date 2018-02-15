# `drift-detection.inc`

[![sampctl](https://shields.southcla.ws/badge/sampctl-drift--detection-2f2f2f.svg?style=for-the-badge)](https://github.com/ltkarim/drift-detection)

>
> This package provides basic drift detection functionality that can be used in drift gamemodes or minigames, it is aimed be basic to avoid affecting the servers performance.
>
> Video preview: https://gfycat.com/gifs/detail/ReflectingSeveralArizonaalligatorlizard


## Installation

Simply install to your project:

```bash
sampctl package install ltkarim/drift-detection
```

Include in your code and begin using the library:

```pawn
#include <drift-detection>
```

## Usage

### Functions

```pawn
Drift::EnableDetection(playerid = -1);
Drift::EnableDamageCheck(playerid = -1);

Drift::DisableDetection(playerid = -1);
Drift::DisableDamageCheck(playerid = -1);

bool: Drift::IsDetectionEnabled(playerid = -1);
bool: Drift::IsDamageCheckEnabled(playerid = -1);
bool: Drift::IsPlayerDrifting(playerid);
```

### Callbacks

```pawn
forward	OnPlayerDriftStart(playerid);
forward	OnPlayerDriftUpdate(playerid, Float: drift_angle, Float: speed);
forward	OnPlayerDriftEnd(playerid, reason, Float: distance, time);
```

### Drift End Reasons

* **DRIFT_END_REASON_TIMEOUT**: player has stopped drifting for N * delay interval. 
* **DRIFT_END_REASON_OTHER**: player left his vehicle or disconencted from the server.
* **DRIFT_END_REASON_DAMAGED**: vehicle is damaged (only when DamageCheck is enabled)



## Testing

To test, simply run the package:

```bash
sampctl package run
```

And connect to `localhost:7777` to test.
