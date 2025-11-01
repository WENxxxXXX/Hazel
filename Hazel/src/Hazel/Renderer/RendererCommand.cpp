#include "hzpch.h"
#include "Hazel/Renderer/RendererCommand.h"

namespace Hazel
{

	Scope<RendererAPI> RendererCommand::s_RendererAPI = RendererAPI::Create();

}