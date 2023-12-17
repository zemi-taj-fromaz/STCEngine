#pragma once

#include "Layer.h"
#include <memory>
#include <vector>
#include <stdexcept>

class LayerStack
{
public:
	LayerStack();
	~LayerStack();

	void push_layer(std::shared_ptr<Layer>& layer);
	void pop_layer(std::shared_ptr<Layer>& layer);

	std::vector<std::shared_ptr<Layer>>::iterator begin() { return m_Layers.begin(); }
	std::vector<std::shared_ptr<Layer>>::iterator end() { return m_Layers.end(); }

	inline int get_layer_count() const { return m_LayerCounter; }


	std::shared_ptr<Layer>& operator[](size_t index) {
		// Perform bounds checking if necessary
		if (index >= m_Layers.size()) {
			throw std::out_of_range("Index out of bounds");
		}

		// Return a reference to the element at the specified index
		return m_Layers[index];
	}

private:
	std::vector<std::shared_ptr<Layer>> m_Layers;
	int m_LayerCounter;
	//	std::vector<Layer*>::iterator m_LayerInsert;
};


