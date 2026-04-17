# Unreal Engine 4/5 Sobel Outline Post-Process Material Setup

This document describes how to create a Post-Process Material for edge detection in UE4/UE5.

## 1. Material Setup
1. Create a new Material named `M_PostProcess_SobelOutline`.
2. Open the Material and set the following properties:
   - **Material Domain**: Post Process
   - **Blendable Location**: Before Tonemapping

## 2. Nodes Setup
We need to sample `SceneDepth` and `WorldNormal` using the `SceneTexture` node.

### 2.1 Parameters
Create Scalar/Vector Parameters:
- `OutlineWidth` (Scalar, Default: 1.0)
- `OutlineIntensity` (Scalar, Default: 1.0)
- `OutlineColor` (Vector, Default: (1, 0.5, 0, 1))
- `DepthThreshold` (Scalar, Default: 0.05)
- `NormalThreshold` (Scalar, Default: 0.2)

### 2.2 UV Offsets
To implement a Sobel operator, sample the `SceneTexture` at 8 surrounding pixels.
Use a `ScreenPosition` node combined with `ViewSize` (1/ViewSize) multiplied by `OutlineWidth` to get the pixel offset.
Create offsets for Top-Left, Top, Top-Right, Left, Right, Bottom-Left, Bottom, Bottom-Right.

### 2.3 Sobel Math
Calculate the gradients `Gx` and `Gy` for both Depth and Normal.
- `Gx = (TR + 2*R + BR) - (TL + 2*L + BL)`
- `Gy = (BL + 2*B + BR) - (TL + 2*T + TR)`
- `Edge = sqrt(Gx^2 + Gy^2)`

Combine the Depth Edge and Normal Edge (e.g., using `Max`), apply the thresholds via a `Step` or `SmoothStep` node, and multiply by `OutlineIntensity`.

### 2.4 Final Output
`Lerp` between the original `SceneColor` (SceneTexture: PostProcessInput0) and `OutlineColor`, using the calculated `Edge` as the alpha mask.
Plug the output into the **Emissive Color** of the Material.

## 3. Usage
1. Create a **Material Instance** (`MI_PostProcess_SobelOutline`) from the material.
2. In your level, select the **PostProcessVolume**.
3. Under **Rendering Features > Post Process Materials**, add an element.
4. Choose **Asset reference** and assign `MI_PostProcess_SobelOutline`.
5. Adjust the parameters in the Material Instance to fit the art style.