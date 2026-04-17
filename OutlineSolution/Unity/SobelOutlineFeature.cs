using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

public class SobelOutlineFeature : ScriptableRendererFeature
{
    class SobelOutlinePass : ScriptableRenderPass
    {
        public Material material;
        private RenderTargetIdentifier source;

        public void Setup(RenderTargetIdentifier src)
        {
            this.source = src;
        }

        public override void Execute(ScriptableRenderContext context, ref RenderingData renderingData)
        {
            if (material == null)
                return;

            CommandBuffer cmd = CommandBufferPool.Get("SobelOutline");
            
            RenderTextureDescriptor opaqueDesc = renderingData.cameraData.cameraTargetDescriptor;
            opaqueDesc.depthBufferBits = 0;

            int tempTarget = Shader.PropertyToID("_TempOutlineTarget");
            cmd.GetTemporaryRT(tempTarget, opaqueDesc);
            
            Blit(cmd, source, tempTarget, material);
            Blit(cmd, tempTarget, source);
            
            context.ExecuteCommandBuffer(cmd);
            CommandBufferPool.Release(cmd);
        }
    }

    [System.Serializable]
    public class OutlineSettings
    {
        public Material outlineMaterial = null;
        public RenderPassEvent renderPassEvent = RenderPassEvent.AfterRenderingTransparents;
    }

    public OutlineSettings settings = new OutlineSettings();
    SobelOutlinePass m_ScriptablePass;

    public override void Create()
    {
        m_ScriptablePass = new SobelOutlinePass();
        m_ScriptablePass.renderPassEvent = settings.renderPassEvent;
        m_ScriptablePass.material = settings.outlineMaterial;
    }

    public override void AddRenderPasses(ScriptableRenderer renderer, ref RenderingData renderingData)
    {
        if (settings.outlineMaterial == null)
        {
            Debug.LogWarningFormat("Missing Outline Material. SobelOutlineFeature will not execute. Check your settings.");
            return;
        }

        m_ScriptablePass.Setup(renderer.cameraColorTarget);
        renderer.EnqueuePass(m_ScriptablePass);
    }
}