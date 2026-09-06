#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Core/Base.h"

namespace Vulcanite {
	class Material {
	public:
		~Material() = default;

		virtual void SetColor(const glm::vec4& color) = 0;
		virtual const glm::vec4& GetColor() const = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Material> Create(const std::string& name);
	};
}