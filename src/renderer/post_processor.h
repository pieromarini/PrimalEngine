#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

namespace primal::renderer {

  class Texture;
  class RenderTarget;
  class Shader;
  class Renderer;
  class Camera;

  class PostProcessor {
	public:
	  PostProcessor(Renderer* renderer);
	  ~PostProcessor();

	  // updates all render targets to match new render size
	  void updateRenderSize(unsigned int width, unsigned int height);

	  // process stages
	  void processPreLighting(Renderer* renderer, RenderTarget* gBuffer, Camera* camera);
	  void processPostLighting(Renderer* renderer, RenderTarget* gBuffer, RenderTarget* output, Camera* camera);

	  // blit all combined post-processing steps to default framebuffer
	  void blit(Renderer* renderer, Texture* soruce);

	  Texture* DownSampledHalfOutput;
	  Texture* DownSampledQuarterOutput;
	  Texture* DownSampledEightOutput;
	  Texture* DownSampledSixteenthOutput;
	  Texture* BlurredEightOutput;
	  Texture* BlurredSixteenthOutput;
	  Texture* SSAOOutput;
	  Texture* BloomOutput1;
	  Texture* BloomOutput2;
	  Texture* BloomOutput3;
	  Texture* BloomOutput4;

	  // toggles
	  bool Sepia = false;
	  bool Vignette = true;
	  bool Bloom = true;
	  bool SSAO = true;
	  bool TXAA = false;
	  bool MotionBlur = true;

	  // ssao
	  int SSAOKernelSize = 32;

	  // motion-blur
	  float FPSTarget = 60.0;

	private:
	  // global post-process state
	  Shader* m_PostProcessShader;
	  RenderTarget* m_RTOutput;

	  // ssao
	  RenderTarget* m_SSAORenderTarget;
	  Shader* m_SSAOShader;
	  Shader* m_SSAOBlurShader;
	  Texture* m_SSAONoise;

	  // bloom
	  RenderTarget* m_BloomRenderTarget0;
	  RenderTarget* m_BloomRenderTarget1;
	  RenderTarget* m_BloomRenderTarget2;
	  RenderTarget* m_BloomRenderTarget3;
	  RenderTarget* m_BloomRenderTarget4;
	  Shader* m_BloomShader;
	  Shader* m_BloomBlurShader;

	  // downsample
	  RenderTarget* m_DownSampleRTHalf;
	  RenderTarget* m_DownSampleRTQuarter;
	  RenderTarget* m_DownSampleRTEight;
	  RenderTarget* m_DownSampleRTSixteenth;
	  Shader* m_DownSampleShader;

	  // (lower resolution) downsampled blurs
	  RenderTarget* m_DownSampleBlurRTEight;
	  RenderTarget* m_DownSampleBlurRTSixteenth;

	  // blur
	  RenderTarget* m_GaussianRTHalf_H;
	  RenderTarget* m_GaussianRTQuarter_H;
	  RenderTarget* m_GaussianRTEight_H;
	  RenderTarget* m_GaussianRTSixteenth_H;
	  Shader* m_OnePassGaussianShader;

	  Texture* downsample(Renderer* renderer, Texture* src, RenderTarget* dst);
	  Texture* blur(Renderer* renderer, Texture* src, RenderTarget* dst, int count);
  };

}

#endif
