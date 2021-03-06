#pragma once

#include "LightTypes.h"
#include "Material.h"
#include "Effect.h"

namespace Angazi::Graphics
{
	class StandardEffect : public Effect
	{
	public:
		META_CLASS_DECLARE;

		StandardEffect() :Effect(EffectType::StandardType) {};
		~StandardEffect() = default;
		StandardEffect(const StandardEffect&) = delete;
		StandardEffect& operator=(const StandardEffect&) = delete;

		void Initialize(const std::filesystem::path& fileName = "../../Assets/Shaders/Standard.fx") override;
		void Terminate() override;

		void Begin() override;
		void End() override;

	public:

		void SetWorldMatrix(const Math::Matrix4& world);
		void SetWVPMatrix(const Math::Matrix4& world, const Math::Matrix4& view, const Math::Matrix4& projection);
		void SetViewPosition(const Math::Vector3& viewPosition);
		void SetTransformData(const Math::Matrix4& world, const Math::Matrix4& view, const Math::Matrix4& projection, const Math::Vector3& viewPosition);

		void SetDirectionalLight(const DirectionalLight& light);
		void SetMaterial(const Material& material);

		void SetBoneTransforms(const std::vector<Math::Matrix4>& boneTransforms);

		void SetClippingPlane(const Math::Vector4& plane);

		void SetDiffuseTexture(const std::filesystem::path& fileName, bool enableGammaCorrection = false);
		void SetNormalTexture(const std::filesystem::path& fileName);
		void SetSpecularTexture(const std::filesystem::path& fileName);
		void SetDisplacementTexture(const std::filesystem::path& fileName);
		void SetAOTexture(const std::filesystem::path& fileName);
		void SetDepthTexture(const RenderTarget* target);

		void SetDiffuseTexture(const Texture* diffuseTexture);
		void SetNormalTexture(const Texture* normalTexture);
		void SetSpecularTexture(const Texture* specularTexture);
		void SetDisplacementTexture(const Texture* displacementTexture);
		void SetAOTexture(const Texture* aoTexture);
		void SetDepthTexture(const Texture* depthTexture);

		void SetSpecularMapWeight(float weight) { mSettings.specularMapWeight = weight; }
		void SetBumpMapWeight(float weight) { mSettings.bumpMapWeight = weight; }
		void SetNormalMapWeight(float weight) { mSettings.normalMapWeight = weight; }
		void SetAOWeight(float weight) { mSettings.aoMapWeight = weight; }
		void SetBrightness(float brightness) { mSettings.brightness = brightness; };
		void SetDepthBias(float bias) { mSettings.depthBias = bias; };

		void UseShadow(bool use) { mSettings.useShadow = use == true ? 1 : 0; };
		void SetSkinnedMesh(bool isSkinnedMesh) { mSettings.isSkinnedMesh = isSkinnedMesh == true ?  1.0f : 0.0f; };

		void UpdateShadowBuffer(const Math::Matrix4& mat) { mShadowConstantBuffer.Update(&mat); };
		void UpdateSettings() { mSettingsBuffer.Update(&mSettings); };

	private:
		struct TransformData
		{
			Angazi::Math::Matrix4 world;
			Angazi::Math::Matrix4 wvp;
			Angazi::Math::Vector3 viewPosition;
			float padding = 0.0f;
		};

		struct Settings
		{
			float specularMapWeight = 0.0f;
			float bumpMapWeight = 0.0f;
			float normalMapWeight = 0.0f;
			float aoMapWeight = 0.0f;
			float brightness = 1.0f;
			int useShadow = 0;
			float depthBias = 0.0003f;
			float isSkinnedMesh = 0.0f;
			//float padding[3];
		};

		struct BoneTransform
		{
			Angazi::Math::Matrix4 boneTransforms[256];
		};

		struct Clipping
		{
			Angazi::Math::Vector4 plane = 0.0f;
		};

		using TransformBuffer = Angazi::Graphics::TypedConstantBuffer<TransformData>;
		using LightBuffer = Angazi::Graphics::TypedConstantBuffer<Angazi::Graphics::DirectionalLight>;
		using MaterialBuffer = Angazi::Graphics::TypedConstantBuffer<Angazi::Graphics::Material>;
		using SettingsBuffer = Angazi::Graphics::TypedConstantBuffer<Settings>;
		using ShadowConstantBuffer = Angazi::Graphics::TypedConstantBuffer<Angazi::Math::Matrix4>;
		using BoneTransformBuffer = Angazi::Graphics::TypedConstantBuffer<BoneTransform>;
		using ClippingConstantBuffer = Angazi::Graphics::TypedConstantBuffer<Clipping>;

		TransformBuffer mTransformBuffer;
		LightBuffer mLightBuffer;
		MaterialBuffer mMaterialBuffer;
		SettingsBuffer mSettingsBuffer;
		BoneTransformBuffer mBoneTransformBuffer;
		ClippingConstantBuffer mClippingConstantBuffer;

		Settings mSettings;
		TransformData transformData;
		BoneTransform mBoneTransform;
		Clipping mClipping;

		Angazi::Graphics::VertexShader mVertexShader;
		Angazi::Graphics::PixelShader  mPixelShader;

		Angazi::Graphics::BlendState mBlendState;

		Angazi::Graphics::Texture mDiffuseMap;
		Angazi::Graphics::Texture mSpecularMap;
		Angazi::Graphics::Texture mDisplacementMap;
		Angazi::Graphics::Texture mNormalMap;
		Angazi::Graphics::Texture mAmbientOcclusionMap;

		//Shadow
		ShadowConstantBuffer mShadowConstantBuffer;
	};
}