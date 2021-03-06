#ifndef _CPPUTIL_ENGINE_MATERIAL_BSDF_FROSTBITE_H_
#define _CPPUTIL_ENGINE_MATERIAL_BSDF_FROSTBITE_H_

#include <CppUtil/Engine/BSDF.h>
#include <CppUtil/Engine/GGX.h>

namespace CppUtil {
	namespace Engine {
		class BSDF_Frostbite : public BSDF {
		public:
			BSDF_Frostbite(const RGBf & colorFactor = RGBf(1.f), float roughnessFactor = 1.f, float metallicFactor = 1.f)
				: colorFactor(colorFactor), roughnessFactor(roughnessFactor), metallicFactor(metallicFactor),
				albedoTexture(nullptr), metallicTexture(nullptr), aoTexture(nullptr), roughnessTexture(nullptr), normalTexture(nullptr) { }

		public:
			static const Basic::Ptr<BSDF_Frostbite> New(const RGBf & colorFactor = RGBf(1.f), float roughnessFactor = 1.f, float metallicFactor = 1.f) {
				return Basic::New<BSDF_Frostbite>(colorFactor, roughnessFactor, metallicFactor);
			}

		protected:
			virtual ~BSDF_Frostbite() = default;

		public:
			virtual const RGBf F(const Normalf & wo, const Normalf & wi, const Point2 & texcoord) override;

			// probability density function
			virtual float PDF(const Normalf & wo, const Normalf & wi, const Point2 & texcoord) override;

			// PD is probability density
			// return albedo
			virtual const RGBf Sample_f(const Normalf & wo, const Point2 & texcoord, Normalf & wi, float & PD) override;

			virtual void ChangeNormal(const Point2 & texcoord, const Normalf & tangent, Normalf & normal) const override;

		private:
			// Fresnel
			static const RGBf Fr(const Normalf & w, const Normalf & h, const RGBf & albedo, float metallic);

			const RGBf GetAlbedo(const Point2 & texcoord) const;
			float GetMetallic(const Point2 & texcoord) const;
			float GetRoughness(const Point2 & texcoord) const;
			float GetAO(const Point2 & texcoord) const;

			static const Vec3f Fr_DisneyDiffuse(const Normalf & wo, const Normalf & wi, float linearRoughness);

		public:
			GGX ggx;

			RGBf colorFactor;
			Basic::Ptr<Basic::Image> albedoTexture;

			// 0--1
			float metallicFactor;
			Basic::Ptr<Basic::Image> metallicTexture;

			// 0--1
			float roughnessFactor;
			Basic::Ptr<Basic::Image> roughnessTexture;

			Basic::Ptr<Basic::Image> aoTexture; // 只用与实时渲染

			Basic::Ptr<Basic::Image> normalTexture;
		};
	}
}


#endif //!_CPPUTIL_ENGINE_MATERIAL_BSDF_FROSTBITE_H_