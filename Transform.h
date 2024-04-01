#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Transform {

public:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::vec3 axisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);
	float angle = 0.0f;

	glm::mat4* TransformMatrix() {

		front = rotateVector(front, axisOfRotation, angle);
		up = rotateVector(up, axisOfRotation, angle);
		right = rotateVector(right, axisOfRotation, angle);

		transformMatrix = glm::scale(
			glm::rotate(
				glm::translate(
					glm::mat4(1.0f), position),
				glm::radians(angle), axisOfRotation),
			scale);

		return &transformMatrix;
	}



	glm::mat4* CalculateTransformMatr() {

		transformMatrix = glm::mat4(1.0f);

		//TRANSLATE
		transformMatrix = glm::translate(transformMatrix, position);

		//ROTATE
		rotQuat = glm::quat(glm::radians(rotation));

		front = rotQuat * worldFront;
		up = rotQuat * worldUp;
		right = rotQuat * worldRight;

		rotMt = glm::mat4_cast(rotQuat);
		transformMatrix = transformMatrix * rotMt;

		//SCALE
		transformMatrix = glm::scale(transformMatrix, scale);
		return &transformMatrix;
	}

	glm::mat4* CalculateTransformMatr(glm::vec3 _rotation) {

		transformMatrix = glm::mat4(1.0f);

		//TRANSLATE
		transformMatrix = glm::translate(transformMatrix, position);

		//ROTATE
		rotQuat = glm::quat(glm::radians(_rotation));

		front = rotQuat * worldFront;
		up = rotQuat * worldUp;
		right = rotQuat * worldRight;

		rotMt = glm::mat4_cast(rotQuat);
		transformMatrix = transformMatrix * rotMt;

		//SCALE
		transformMatrix = glm::scale(transformMatrix, scale);
		return &transformMatrix;
	}


	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 worldFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 worldRight = glm::vec3(1.0f, 0.0f, 0.0f);

	static glm::vec3 rotateVector(const glm::vec3& vectorToRotate, const glm::vec3& axisOfRotation, float angleInDegrees) {

		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angleInDegrees), axisOfRotation);
		glm::vec4 rotatedVector = rotationMatrix * glm::vec4(vectorToRotate, 1.0f);
		return glm::vec3(rotatedVector);
	}

private:

	glm::mat4 transformMatrix = glm::mat4(1.0f);
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	glm::quat rotQuat;
	glm::mat4 rotMt;

	
};