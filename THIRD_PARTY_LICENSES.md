# Third Party Licenses

AgentOS integrates with and depends on several amazing open-source projects. 
This document lists the open-source software libraries and models used in this project and their respective licenses.

## 1. JUCE Framework
- **License**: GPLv3 / Commercial / JUCE Personal License
- **Usage**: GUI Application, Audio Plugin Wrappers, Window Management
- **Link**: [https://juce.com/](https://juce.com/)
- *Note: If AgentOS is released as closed-source commercial software in the future, a commercial JUCE license must be acquired.*

## 2. llama.cpp
- **License**: MIT License
- **Usage**: High-performance local inference backend for GGUF/GGML models.
- **Link**: [https://github.com/ggerganov/llama.cpp](https://github.com/ggerganov/llama.cpp)

## 3. SQLite
- **License**: Public Domain
- **Usage**: Internal storage for MemoryEngine and Governance Logs.
- **Link**: [https://www.sqlite.org/](https://www.sqlite.org/)

## 4. ONNX Runtime (Optional/Future)
- **License**: MIT License
- **Usage**: Execution of machine learning models for vision and OCR tasks.
- **Link**: [https://onnxruntime.ai/](https://onnxruntime.ai/)

## 5. Hugging Face Models
Models downloaded via the AgentOS Plugin Marketplace or ModelTool may carry their own specific licenses (e.g., Apache 2.0, Llama-2 License, MIT). Please refer to the specific model card on Hugging Face for usage restrictions.
