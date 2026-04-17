# Automation Tests: Outline Stability

This document describes the test cases designed to ensure the stability of the Edge Detection (Sobel) Outline rendering scheme across various engine states.

## Test 1: Transform Stability
- **Objective**: Verify that the outline remains consistent when the object is translated, rotated, and scaled.
- **Steps**:
  1. Select an object (e.g., a simple Cube or the complex Spider character).
  2. Enable the outline.
  3. Translate the object across the screen.
  4. Rotate the object 360 degrees on all axes.
  5. Scale the object from 0.1x to 10x.
- **Expected Result**: The outline must accurately wrap the object at all times without detaching, tearing, or exhibiting the "double-size" effect seen in the legacy stencil implementation.

## Test 2: Camera Proximity & LOD
- **Objective**: Ensure the outline thickness (in pixels) remains constant regardless of distance to the camera.
- **Steps**:
  1. Select an object with the outline enabled.
  2. Move the camera from close proximity to a far distance.
  3. If the object has LODs, observe the transition points.
- **Expected Result**: The outline width on screen must match the `Outline Width` parameter exactly. It should not shrink or expand wildly.

## Test 3: Animation (Skinned Meshes)
- **Objective**: Confirm that the outline dynamically updates with skeletal animation.
- **Steps**:
  1. Load the Spider character with a walk cycle animation.
  2. Select the Spider to enable the outline.
  3. Play the animation at various speeds (0.5x, 1x, 2x).
- **Expected Result**: The outline must follow the deforming mesh perfectly per frame. No trailing artifacts or ghosting.

## Test 4: Occlusion & Depth
- **Objective**: Verify that the outline respects scene depth and occlusion correctly (or draws over as intended, depending on configuration).
- **Steps**:
  1. Place an outlined object partially behind a non-outlined object.
  2. Move the camera to change the occlusion perspective.
- **Expected Result**: In our EntityID post-process approach, the outline should be drawn on top (X-Ray style) but correctly masked by the object's own silhouette. In the Unity/UE Depth/Normal approach, the outline should correctly trace visible edges.

## Test 5: Resolution Scaling
- **Objective**: Verify performance and visual consistency across resolutions.
- **Steps**:
  1. Render the scene at 720p, 1080p, and 4K.
  2. Measure the GPU time taken by the Outline Pass.
- **Expected Result**: The outline must look correct at all resolutions. Performance must stay under 1ms at 1080p and 4K on the target hardware.