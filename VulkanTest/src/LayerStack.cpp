#include "LayerStack.h"

LayerStack::LayerStack() : m_LayerCounter(0)
{
	//m_LayerInsert = m_Layers.begin();
}

LayerStack::~LayerStack()
{

}


void LayerStack::push_layer(std::shared_ptr<Layer>& layer)
{
	m_Layers.push_back(layer);

	////	m_LayerInsert = m_Layers.emplace(m_LayerInsert, layer);
	//m_Layers.insert(m_Layers.begin() + m_LayerCounter, layer);
	m_LayerCounter++;
}


void LayerStack::pop_layer(std::shared_ptr<Layer>& layer)
{
	m_Layers.pop_back();
	m_LayerCounter--;
}


