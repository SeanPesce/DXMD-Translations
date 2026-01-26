# Language Translation With LLM/AI  
**Author: Sean Pesce**  

## Disclaimer

Although Large Language Models (LLMs) can be extremely useful for translating large data sets, the output will inevitably contain a high number of
mistranslations (especially when using variants with lower-bit quantization, such as [`translategemma-4b-it`](https://huggingface.co/google/translategemma-4b-it)).
LLM output should always be reviewed by a human (preferably someone who is fluent in both languages).  

## Setting Up (LM Studio)

* Install [LM Studio](https://lmstudio.ai/), [Open WebUI](https://github.com/open-webui/open-webui) (optional), and the [OpenAI Python library](https://github.com/openai/openai-python)
* In LM Studio:
  * Download your preferred variant of Google's [TranslateGemma](https://ollama.com/library/translategemma)
  * Configure TranslateGemma with my custom [Jinja prompt template](https://github.com/SeanPesce/DXMD-Translations/blob/master/rsrc/llm/src/prompt-template.jinja?raw=true)
  * If necessary, configure the LM Studio and Open WebUI servers to be available over the network
* Download the [English language set](https://github.com/SeanPesce/DXMD-Translations/releases/tag/0_translations)
* Download the LLM translator script ([`llm-translate.py`](https://github.com/SeanPesce/DXMD-Translations/blob/master/rsrc/llm/src/llm-translate.py?raw=true))
* Configure the script. The following example shows how to configure the script for use with Open WebUI:

```python
# =================== CONFIG ====================
ORIGINAL_LANGUAGE_FILE = 'C:\\Users\\User\\Downloads\\en.json'
LANGUAGE_SHORT = 'el'  # Greek
BASE_URL = 'http://192.168.1.123:1234/api'
API_KEY = 'sk-00000000000000000000000000000000'  # For example, an Open WebUI API key
MODEL_NAME = 'translategemma-4b-it'  # The identifier of the specific model variant in use
TRANSLATOR = 'JC Denton'  # Your name
TRANSLATOR_CONTACT = 'jcd@unatco.gov'
```

* Run the script: `python3 llm-translate.py`  

There are approximately 100,000 strings to translate (~70,000 after removing duplicates and non-language strings), so depending on your setup (hardware, model, etc.)
it could take hours (or days) to finish. Strings that fail to translate will be `null` in the output JSON file. If translation hangs on a string, you can try unloading
the model in LM Studio (it will be automatically reloaded when the script retries the translation).

## Example Output

<p align="center">
<img align="center" title="Example output from the LLM translation Python script (in progress)" src="https://github.com/SeanPesce/DXMD-Translations/blob/master/image/llm-translate-output-01.png?raw=true" alt="Example output from the LLM translation Python script (in progress)" width="75%">
<br><br>
<img align="center" title="Example output from the LLM translation Python script (finished)" src="https://github.com/SeanPesce/DXMD-Translations/blob/master/image/llm-translate-output-02.png?raw=true" alt="Example output from the LLM translation Python script (finished)" width="75%">
</p>  
