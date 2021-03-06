#ifndef _SUN_H_
#define _SUN_H_

#include "include.h"
#include "texture.h"
#include "camera.h"
#include "sunShader.h"


class Sun
{

public:

	Sun() = delete;
	Sun(const Sun &) = delete;
	Sun & operator=(const Sun &) = delete;

	Sun(TexturePtr texture, const glm::vec3 & lightDirection, float scale)
	{
		_texture = texture;
		_lightDirection = glm::normalize(lightDirection);
		_scale = scale;
	}

	Sun(const char * textureFileName, const glm::vec3 & lightDirection,
			float scale)
		: _texture(new Texture(textureFileName))
	{
		_lightDirection = glm::normalize(lightDirection);
		_scale = scale;
	}

	TexturePtr getTexture() const {
		return _texture;
	}

	void setTexture(TexturePtr texture) {
		_texture = texture;
	}

	glm::vec3 getLightDirection() const {
		return _lightDirection;
	}

	void setLightDirection(const glm::vec3 & lightDirection) {
		_lightDirection = glm::normalize(lightDirection);
	}

	void setScale(float scale) {
		_scale = scale;
	}

	void setPosition(glm::vec3 position) {
		_position = position;
	}

	glm::vec3 getPosition() const
	{
		return _position;
	}

	glm::vec3 getPosition(const Camera * camera) const
	{
		static const float SUN_DISTANCE = 5.0f;
		glm::vec3 position = camera->getPosition() - SUN_DISTANCE * _lightDirection;
		return position;
	}


	void bindGL() const {
		_texture->bindGL();
	}

	void unbindGL() const {
		_texture->unbindGL();
	}

private:

	glm::vec3 _position;

	TexturePtr _texture;

	glm::vec3 _lightDirection;

	float _scale;

}; // class Sun

typedef shared_ptr<Sun> SunPtr;



class SunRenderer
{

public:

	SunRenderer(SunShader * shader, const glm::mat4 & projectionMatrix);

	~SunRenderer();

	void render(const SunPtr & sun, const Camera * camera);

private:

	glm::mat4 calculateModelViewMatrix(const SunPtr & sun, const Camera * camera);

	glm::vec2 getScreenCoords(const glm::vec3 & position, const Camera * camera);

	glm::vec2 getScreenCoords(const glm::vec3 & position, const glm::mat4 & viewMatrix);

	void renderFlare(const SunPtr & sun, const Camera * camera);

	void initGL();

	void initFlare();

private:

	SunShader * _shader;

	// glm::mat4 _projectionMatrix;

	struct FlareTexture{
		TexturePtr texture;
		float scale;
		glm::vec2 screenCoords;

		FlareTexture() {}

		FlareTexture(TexturePtr t, float s, glm::vec2 v)
			: texture(t), scale(s), screenCoords(v) {}

		FlareTexture(Texture * t, float s, glm::vec2 v)
			: texture(TexturePtr(t)), scale(s), screenCoords(v) {}
	};

	FlareTexture * _flares;

	GLuint _VAO;
	GLuint _VBO;

}; // class SunRenderer


#endif