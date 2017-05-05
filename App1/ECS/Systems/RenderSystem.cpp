#include "pch.h"
#include "RenderSystem.h"

#include "..\Components\ModelComponents.h"
#include "..\Components\TransformComponent.h"
#include "..\Components\LightComponent.h"

using namespace ECS::Systems;

RenderSystem::RenderSystem(GLsizei windowWidth, GLsizei windowHeight)
{
	this->renderer = new DeferredRenderer(windowWidth, windowHeight);
	this->postProcessing = new PostProcessing(windowWidth, windowHeight);
	this->gammaPost = new GammaPostProcessing();
}

RenderSystem::~RenderSystem()
{
	delete this->renderer;
	delete this->postProcessing;
	delete this->gammaPost;
}

void RenderSystem::Update(ECS::World & world, const AppTime & time)
{
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	glm::mat4 view, proj, viewProj;
	view = glm::lookAt(glm::vec3(0.0f, 1.0f, 3.0f), glm::vec3(0.0f, 1.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	proj = glm::perspective(45.0f, 1280.f / 720.f, 0.1f, 100.0f);
	viewProj = proj * view;
	GLint worldMatrixLocation = this->renderer->GetWorldMatrixLocation();
	this->renderer->StartGeometryPass(viewProj);
	auto geometryShader = this->renderer->GetGeometryShader();

	for (auto * it : world.GetEntities())
	{
		auto mesh = it->GetComponent<ECS::Components::MeshComponent<VertexType>>();
		auto material = it->GetComponent<ECS::Components::MaterialComponent>();
		if (mesh != NULL && material != NULL)
		{
			auto transform = it->GetComponent<ECS::Components::TransformComponent>()->GetWorldTransform();
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, glm::value_ptr(transform));
			material->material->Use(geometryShader);
			mesh->mesh->Draw();
		}
	}

	this->renderer->EndGeometryPass();
	this->postProcessing->BindFramebuffer();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	this->renderer->StartLightPass();

	glm::mat4 invViewProj = glm::inverse(viewProj);
	glm::vec3 camPos = glm::vec3(0.0f, 1.0f, 3.0f);
	for (auto * it : world.GetEntities())
	{
		auto light = it->GetComponent<ECS::Components::LightComponent>();
		if (light != NULL) light->light->Draw(invViewProj, camPos);
	}

	this->renderer->EndLightPass();
	this->postProcessing->BindFramebuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	this->gammaPost->Draw();
}