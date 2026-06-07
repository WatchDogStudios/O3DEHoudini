# O3DEHoudini

O3DEHoudini integrates SideFX Houdini functionality into the engine, providing tools to import, cook, and manage Houdini Digital Assets (HDAs) and provide runtime interaction with procedurally generated content.

## Features

- Import and instantiate Houdini Digital Assets (HDA)
- Cook and recook HDAs at runtime or in-editor
- Expose HDA parameters to the engine for animation and gameplay
- Support for procedural geometry, materials, and attributes
- Asset caching and optimization for performance

## Requirements

- SideFX Houdini (Houdini Engine 6.0+) with a VALID LICENSE.
- Houdini Engine API/Runtime available on the build machine

## Installation

1. Copy the HoudiniEngine gem folder into the engine's `Gems` directory (already placed at `Gems/HoudiniEngine`).
2. Enable the gem in your project's gem manifest or engine config.
3. Ensure the Houdini Engine runtime is installed and environment variables (if any) are configured.

## License

O3DEHoudini is under the MIT License, but **It requires the Houdini Engine, which needs its own license.** please refer to [SideFX Houdini](https://www.sidefx.com/products/houdini/) for more information. See SideFX License for the license of the Houdini C/C++ API, and any SideFX Products or Assets; Which is required for this Gem to operate.

## SideFX License

```
Copyright (c) 2021 Side Effects Software Inc. All rights reserved.

Redistribution and use of in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

The names Side Effects Software and SideFX may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```