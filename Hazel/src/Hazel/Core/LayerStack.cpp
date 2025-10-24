#include "hzpch.h"
#include "LayerStack.h"
#include "Application.h"

namespace Hazel {
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		for each(Layer * layer in m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto iter = std::find(m_Layers.begin(), 
			m_Layers.begin() + m_LayerInsertIndex, layer);
		if (iter != m_Layers.begin() + m_LayerInsertIndex)
		{
			layer->OnDetach();
			m_Layers.erase(iter);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PushOverLay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopOverLay(Layer* overlay)
	{
		auto iter = std::find(m_Layers.begin() + m_LayerInsertIndex, 
			m_Layers.end(), overlay);
		if (iter != m_Layers.end())
		{
			overlay->OnDetach();
			m_Layers.erase(iter);
		}
	}

}