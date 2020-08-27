#include <random>

#include "post_processor.h"

#include "render_target.h"
#include "resources/resources.h"
#include "core/math/vector3.h"
#include "core/math/vector2.h"

namespace primal::renderer {

  PostProcessor::PostProcessor(Renderer* renderer) {
	// global post-processing shader
	{
	  m_PostProcessShader = Resources::loadShader("post process", "shaders/screen_quad.vs", "shaders/post_processing.fs");
	  m_PostProcessShader->use();
	  m_PostProcessShader->setInt("TexSrc", 0);
	  m_PostProcessShader->setInt("TexBloom1", 1);
	  m_PostProcessShader->setInt("TexBloom2", 2);
	  m_PostProcessShader->setInt("TexBloom3", 3);
	  m_PostProcessShader->setInt("TexBloom4", 4);
	  m_PostProcessShader->setInt("gMotion", 5);
	}
	// down sample
	{
	  m_DownSampleRTHalf = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_DownSampleRTQuarter = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_DownSampleRTEight = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_DownSampleRTSixteenth = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  DownSampledHalfOutput = m_DownSampleRTHalf->getColorTexture(0);
	  DownSampledQuarterOutput = m_DownSampleRTQuarter->getColorTexture(0);
	  DownSampledEightOutput = m_DownSampleRTEight->getColorTexture(0);
	  DownSampledSixteenthOutput = m_DownSampleRTSixteenth->getColorTexture(0);

	  m_DownSampleShader = Resources::loadShader("down sample", "shaders/screen_quad.vs", "shaders/post/down_sample.fs");
	  m_DownSampleShader->use();
	  m_DownSampleShader->setInt("TexSrc", 0);
	}
	// lower resolution downsample blurs
	{
	  m_DownSampleBlurRTEight = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_DownSampleBlurRTSixteenth = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  BlurredEightOutput = m_DownSampleBlurRTEight->getColorTexture(0);
	  BlurredSixteenthOutput = m_DownSampleBlurRTSixteenth->getColorTexture(0);
	}
	// gaussian blur shader
	{
	  m_GaussianRTHalf_H = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_GaussianRTQuarter_H = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_GaussianRTEight_H = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_GaussianRTSixteenth_H = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);

	  m_OnePassGaussianShader = Resources::loadShader("gaussian blur", "shaders/screen_quad.vs", "shaders/post/blur_guassian.fs");
	  m_OnePassGaussianShader->use();
	  m_OnePassGaussianShader->setInt("TexSrc", 0);
	}
	// ssao
	{
	  m_SSAORenderTarget = new RenderTarget(1280, 720, GL_HALF_FLOAT, 1, false);
	  SSAOOutput = m_SSAORenderTarget->getColorTexture(0);

	  m_SSAOShader = Resources::loadShader("ssao", "shaders/screen_quad.vs", "shaders/post/ssao.fs");
	  m_SSAOShader->use();
	  m_SSAOShader->setInt("gPositionMetallic", 0);
	  m_SSAOShader->setInt("gNormalRoughness", 1);
	  m_SSAOShader->setInt("texNoise", 2);

	  std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	  std::default_random_engine generator;
	  std::vector<math::Vector3> ssaoKernel;
	  for (int i = 0; i < SSAOKernelSize; ++i) {
		math::Vector3 sample(
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator));
		sample.normalize();
		sample = sample * randomFloats(generator);
		float scale = (float)i / (float)SSAOKernelSize;
		scale = math::Util::lerp(0.1f, 1.0f, scale * scale);
		sample = sample * scale;
		ssaoKernel.push_back(sample);
	  }
	  std::vector<math::Vector3> ssaoNoise;
	  for (unsigned int i = 0; i < 16; i++) {
		math::Vector3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	  }
	  m_SSAONoise = new Texture();
	  m_SSAONoise->generate(4, 4, GL_RGBA16F, GL_RGB, GL_HALF_FLOAT, &ssaoNoise[0]);

	  m_SSAOShader->setVectorArray("kernel", ssaoKernel.size(), ssaoKernel);
	  m_SSAOShader->setInt("sampleCount", SSAOKernelSize);
	}

	{
	  m_BloomRenderTarget0 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_BloomRenderTarget1 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_BloomRenderTarget2 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_BloomRenderTarget3 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  m_BloomRenderTarget4 = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, false);
	  BloomOutput1 = m_BloomRenderTarget1->getColorTexture(0);
	  BloomOutput2 = m_BloomRenderTarget2->getColorTexture(0);
	  BloomOutput3 = m_BloomRenderTarget3->getColorTexture(0);
	  BloomOutput4 = m_BloomRenderTarget4->getColorTexture(0);

	  m_BloomShader = Resources::loadShader("bloom", "shaders/screen_quad.vs", "shaders/post/bloom.fs");
	  m_SSAOShader->use();
	  m_SSAOShader->setInt("HDRScene", 0);
	}
  }

  PostProcessor::~PostProcessor() {
	delete m_DownSampleRTHalf;
	delete m_DownSampleRTQuarter;
	delete m_DownSampleRTEight;
	delete m_DownSampleRTSixteenth;
	delete m_DownSampleBlurRTEight;
	delete m_DownSampleBlurRTSixteenth;
	delete m_GaussianRTHalf_H;
	delete m_GaussianRTQuarter_H;
	delete m_GaussianRTEight_H;
	delete m_GaussianRTSixteenth_H;
	delete m_SSAONoise;
	delete m_SSAORenderTarget;
	delete m_BloomRenderTarget0;
	delete m_BloomRenderTarget1;
	delete m_BloomRenderTarget2;
	delete m_BloomRenderTarget3;
	delete m_BloomRenderTarget4;
  }

  void PostProcessor::updateRenderSize(unsigned int width, unsigned int height) {

	m_DownSampleRTHalf->resize((int)(width * 0.5f), (int)(height * 0.5f));
	m_DownSampleRTQuarter->resize((int)(width * 0.25f), (int)(height * 0.25f));
	m_DownSampleRTEight->resize((int)(width * 0.125f), (int)(height * 0.125f));
	m_DownSampleRTSixteenth->resize((int)(width * 0.0675f), (int)(height * 0.0675f));

	m_DownSampleBlurRTEight->resize((int)(width * 0.125f), (int)(height * 0.125f));
	m_DownSampleBlurRTSixteenth->resize((int)(width * 0.0675f), (int)(height * 0.0675f));

	m_GaussianRTHalf_H->resize((int)(width * 0.5f), (int)(height * 0.5f));
	m_GaussianRTQuarter_H->resize((int)(width * 0.25f), (int)(height * 0.25f));
	m_GaussianRTEight_H->resize((int)(width * 0.125f), (int)(height * 0.125f));
	m_GaussianRTSixteenth_H->resize((int)(width * 0.0675f), (int)(height * 0.0675f));

	m_BloomRenderTarget0->resize((int)(width * 0.5f), (int)(height * 0.5f));
	m_BloomRenderTarget1->resize((int)(width * 0.5f), (int)(height * 0.5f));
	m_BloomRenderTarget2->resize((int)(width * 0.25f), (int)(height * 0.25f));
	m_BloomRenderTarget3->resize((int)(width * 0.125f), (int)(height * 0.125f));
	m_BloomRenderTarget4->resize((int)(width * 0.0675f), (int)(height * 0.0675f));

	m_SSAORenderTarget->resize((int)(width * 0.5f), (int)(height * 0.5f));
  }

  void PostProcessor::processPreLighting(Renderer* renderer, RenderTarget* gBuffer, Camera* camera) {
	// ssao
	if (SSAO) {
	  gBuffer->getColorTexture(0)->Bind(0);
	  gBuffer->getColorTexture(1)->Bind(1);
	  m_SSAONoise->Bind(2);

	  m_SSAOShader->use();
	  m_SSAOShader->SetVector("renderSize", renderer->GetRenderSize());
	  m_SSAOShader->SetMatrix("projection", camera->Projection);
	  m_SSAOShader->SetMatrix("view", camera->View);

	  glBindFramebuffer(GL_FRAMEBUFFER, m_SSAORenderTarget->ID);
	  glViewport(0, 0, m_SSAORenderTarget->Width, m_SSAORenderTarget->Height);
	  glClear(GL_COLOR_BUFFER_BIT);
	  renderer->renderMesh(renderer->m_NDCPlane, m_SSAOShader);
	}
  }
  // --------------------------------------------------------------------------------------------
  void PostProcessor::processPostLighting(Renderer* renderer, RenderTarget* gBuffer, RenderTarget* output, Camera* camera) {
	// downsample
	{
	  downsample(renderer, output->getColorTexture(0), m_DownSampleRTHalf);
	  downsample(renderer, DownSampledHalfOutput, m_DownSampleRTQuarter);
	  downsample(renderer, DownSampledQuarterOutput, m_DownSampleRTEight);
	  downsample(renderer, DownSampledEightOutput, m_DownSampleRTSixteenth);
	}
	// blur (only lower resolution) down-sampled textures (for glass refraction/ssr-glossy)
	{
	  blur(renderer, DownSampledEightOutput, m_DownSampleBlurRTEight, 4);
	  blur(renderer, DownSampledSixteenthOutput, m_DownSampleBlurRTSixteenth, 4);
	}
	// bloom
	if (Bloom) {
	  m_BloomShader->use();
	  output->getColorTexture(0)->Bind(0);

	  glBindFramebuffer(GL_FRAMEBUFFER, m_BloomRenderTarget0->ID);
	  glViewport(0, 0, m_BloomRenderTarget0->Width, m_BloomRenderTarget0->Height);
	  glClear(GL_COLOR_BUFFER_BIT);
	  renderer->renderMesh(renderer->m_NDCPlane, m_BloomShader);

	  // blur bloom result
	  blur(renderer, m_BloomRenderTarget0->getColorTexture(0), m_BloomRenderTarget1, 8);
	  blur(renderer, m_BloomRenderTarget1->getColorTexture(0), m_BloomRenderTarget2, 8);
	  blur(renderer, m_BloomRenderTarget2->getColorTexture(0), m_BloomRenderTarget3, 8);
	  blur(renderer, m_BloomRenderTarget3->getColorTexture(0), m_BloomRenderTarget4, 8);
	}
  }

  void PostProcessor::blit(Renderer* renderer, Texture* source) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, renderer->GetRenderSize().x, renderer->GetRenderSize().y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// bind input texture data
	source->Bind(0);
	BloomOutput1->Bind(1);
	BloomOutput2->Bind(2);
	BloomOutput3->Bind(3);
	BloomOutput4->Bind(4);
	renderer->m_GBuffer->getColorTexture(3)->bind(5);

	// set settings
	m_PostProcessShader->use();
	m_PostProcessShader->setBool("SSAO", SSAO);
	m_PostProcessShader->setBool("Sepia", Sepia);
	m_PostProcessShader->setBool("Vignette", Vignette);
	m_PostProcessShader->setBool("Bloom", Bloom);
	// motion blur
	m_PostProcessShader->setBool("MotionBlur", MotionBlur);
	m_PostProcessShader->setFloat("MotionScale", ImGui::GetIO().Framerate / FPSTarget * 0.8);
	m_PostProcessShader->setInt("MotionSamples", 16);

	renderer->renderMesh(renderer->m_NDCPlane, m_PostProcessShader);
  }
  // --------------------------------------------------------------------------------------------
  Texture* PostProcessor::downsample(Renderer* renderer, Texture* src, RenderTarget* dst) {
	glViewport(0, 0, dst->Width, dst->Height);
	glBindFramebuffer(GL_FRAMEBUFFER, dst->ID);
	glClear(GL_COLOR_BUFFER_BIT);

	src->Bind(0);
	m_DownSampleShader->use();
	renderer->renderMesh(renderer->m_NDCPlane, m_DownSampleShader);

	// output resulting (downsampled) texture
	return dst->getColorTexture(0);
  }
  // --------------------------------------------------------------------------------------------
  Texture* PostProcessor::blur(Renderer* renderer, Texture* src, RenderTarget* dst, int count) {
	assert(count >= 2 && count % 2 == 0);// count must be more than 2 and be even

	// pick pre-defined render targets for blur based on render size
	RenderTarget* rtHorizontal;
	RenderTarget* rtVertical;
	if (dst->Width == m_GaussianRTHalf_H->Width) {
	  rtHorizontal = m_GaussianRTHalf_H;
	} else if (dst->Width == m_GaussianRTQuarter_H->Width) {
	  rtHorizontal = m_GaussianRTQuarter_H;
	} else if (dst->Width == m_GaussianRTEight_H->Width) {
	  rtHorizontal = m_GaussianRTEight_H;
	} else {
	  rtHorizontal = m_GaussianRTSixteenth_H;
	}
	// use destination as vertical render target of ping-pong algorithm
	rtVertical = dst;
	// resize viewport to destination dimensions
	glViewport(0, 0, dst->Width, dst->Height);

	bool horizontal = true;
	m_OnePassGaussianShader->use();
	for (int i = 0; i < count; ++i, horizontal = !horizontal) {
	  m_OnePassGaussianShader->setBool("horizontal", horizontal);
	  if (i == 0) {
		src->Bind(0);
	  } else if (horizontal) {
		rtVertical->getColorTexture(0)->Bind(0);
	  } else if (!horizontal) {
		rtHorizontal->getColorTexture(0)->Bind(0);
	  }
	  glBindFramebuffer(GL_FRAMEBUFFER, horizontal ? rtHorizontal->ID : rtVertical->ID);
	  renderer->renderMesh(renderer->m_NDCPlane, m_OnePassGaussianShader);
	}

	// output resulting (blurred) texture
	return dst->getColorTexture(0);
  }
}// namespace primal::renderer
