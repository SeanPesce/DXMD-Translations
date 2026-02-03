#!/usr/bin/env python3
# Author: Sean Pesce
#
# Translate a DXMD language set using LLMs

import datetime
import json
import os
import re
import sys
import time

from openai import OpenAI


LANGUAGES = {
    'aa': 'Afar',
    'aa-DJ': 'Afar',
    'aa-ER': 'Afar',
    'ab': 'Abkhazian',
    'af': 'Afrikaans',
    'af-NA': 'Afrikaans',
    'ak': 'Akan',
    'am': 'Amharic',
    'an': 'Aragonese',
    'ar': 'Arabic',
    'ar-AE': 'Arabic',
    'ar-BH': 'Arabic',
    'ar-DJ': 'Arabic',
    'ar-DZ': 'Arabic',
    'ar-EG': 'Arabic',
    'ar-EH': 'Arabic',
    'ar-ER': 'Arabic',
    'ar-IL': 'Arabic',
    'ar-IQ': 'Arabic',
    'ar-JO': 'Arabic',
    'ar-KM': 'Arabic',
    'ar-KW': 'Arabic',
    'ar-LB': 'Arabic',
    'ar-LY': 'Arabic',
    'ar-MA': 'Arabic',
    'ar-MR': 'Arabic',
    'ar-OM': 'Arabic',
    'ar-PS': 'Arabic',
    'ar-QA': 'Arabic',
    'ar-SA': 'Arabic',
    'ar-SD': 'Arabic',
    'ar-SO': 'Arabic',
    'ar-SS': 'Arabic',
    'ar-SY': 'Arabic',
    'ar-TD': 'Arabic',
    'ar-TN': 'Arabic',
    'ar-YE': 'Arabic',
    'as': 'Assamese',
    'az': 'Azerbaijani',
    'az-Arab': 'Azerbaijani',
    'az-Arab-IQ': 'Azerbaijani',
    'az-Arab-TR': 'Azerbaijani',
    'az-Cyrl': 'Azerbaijani',
    'az-Latn': 'Azerbaijani',
    'ba': 'Bashkir',
    'be': 'Belarusian',
    'be-tarask': 'Belarusian',
    'bg': 'Bulgarian',
    'bg-BG': 'Bulgarian',
    'bm': 'Bambara',
    'bm-Nkoo': 'Bambara',
    'bn': 'Bengali',
    'bn-IN': 'Bengali',
    'bo': 'Tibetan',
    'bo-IN': 'Tibetan',
    'br': 'Breton',
    'bs': 'Bosnian',
    'bs-Cyrl': 'Bosnian',
    'bs-Latn': 'Bosnian',
    'ca': 'Catalan',
    'ca-AD': 'Catalan',
    'ca-ES': 'Catalan',
    'ca-FR': 'Catalan',
    'ca-IT': 'Catalan',
    'ce': 'Chechen',
    'co': 'Corsican',
    'cs': 'Czech',
    'cs-CZ': 'Czech',
    'cv': 'Chuvash',
    'cy': 'Welsh',
    'da': 'Danish',
    'da-DK': 'Danish',
    'da-GL': 'Danish',
    'de': 'German',
    'de-AT': 'German',
    'de-BE': 'German',
    'de-CH': 'German',
    'de-DE': 'German',
    'de-IT': 'German',
    'de-LI': 'German',
    'de-LU': 'German',
    'dv': 'Divehi',
    'dz': 'Dzongkha',
    'ee': 'Ewe',
    'ee-TG': 'Ewe',
    'el': 'Greek',
    'el-CY': 'Greek',
    'el-GR': 'Greek',
    'el-polyton': 'Greek',
    'en': 'English',
    'en-AE': 'English',
    'en-AG': 'English',
    'en-AI': 'English',
    'en-AS': 'English',
    'en-AT': 'English',
    'en-AU': 'English',
    'en-BB': 'English',
    'en-BE': 'English',
    'en-BI': 'English',
    'en-BM': 'English',
    'en-BS': 'English',
    'en-BW': 'English',
    'en-BZ': 'English',
    'en-CA': 'English',
    'en-CC': 'English',
    'en-CH': 'English',
    'en-CK': 'English',
    'en-CM': 'English',
    'en-CX': 'English',
    'en-CY': 'English',
    'en-CZ': 'English',
    'en-DE': 'English',
    'en-DG': 'English',
    'en-DK': 'English',
    'en-DM': 'English',
    'en-ER': 'English',
    'en-ES': 'English',
    'en-FI': 'English',
    'en-FJ': 'English',
    'en-FK': 'English',
    'en-FM': 'English',
    'en-FR': 'English',
    'en-GB': 'English',
    'en-GD': 'English',
    'en-GG': 'English',
    'en-GH': 'English',
    'en-GI': 'English',
    'en-GM': 'English',
    'en-GS': 'English',
    'en-GU': 'English',
    'en-GY': 'English',
    'en-HK': 'English',
    'en-HU': 'English',
    'en-ID': 'English',
    'en-IE': 'English',
    'en-IL': 'English',
    'en-IM': 'English',
    'en-IN': 'English',
    'en-IO': 'English',
    'en-IT': 'English',
    'en-JE': 'English',
    'en-JM': 'English',
    'en-KE': 'English',
    'en-KI': 'English',
    'en-KN': 'English',
    'en-KY': 'English',
    'en-LC': 'English',
    'en-LR': 'English',
    'en-LS': 'English',
    'en-MG': 'English',
    'en-MH': 'English',
    'en-MO': 'English',
    'en-MP': 'English',
    'en-MS': 'English',
    'en-MT': 'English',
    'en-MU': 'English',
    'en-MV': 'English',
    'en-MW': 'English',
    'en-MY': 'English',
    'en-NA': 'English',
    'en-NF': 'English',
    'en-NG': 'English',
    'en-NL': 'English',
    'en-NO': 'English',
    'en-NR': 'English',
    'en-NU': 'English',
    'en-NZ': 'English',
    'en-PG': 'English',
    'en-PH': 'English',
    'en-PK': 'English',
    'en-PL': 'English',
    'en-PN': 'English',
    'en-PR': 'English',
    'en-PT': 'English',
    'en-PW': 'English',
    'en-RO': 'English',
    'en-RW': 'English',
    'en-SB': 'English',
    'en-SC': 'English',
    'en-SD': 'English',
    'en-SE': 'English',
    'en-SG': 'English',
    'en-SH': 'English',
    'en-SI': 'English',
    'en-SK': 'English',
    'en-SL': 'English',
    'en-SS': 'English',
    'en-SX': 'English',
    'en-SZ': 'English',
    'en-TC': 'English',
    'en-TK': 'English',
    'en-TO': 'English',
    'en-TT': 'English',
    'en-TV': 'English',
    'en-TZ': 'English',
    'en-UG': 'English',
    'en-UM': 'English',
    'en-VC': 'English',
    'en-VG': 'English',
    'en-VI': 'English',
    'en-VU': 'English',
    'en-WS': 'English',
    'en-ZA': 'English',
    'en-ZM': 'English',
    'en-ZW': 'English',
    'eo': 'Esperanto',
    'es': 'Spanish',
    'es-AR': 'Spanish',
    'es-BO': 'Spanish',
    'es-BR': 'Spanish',
    'es-BZ': 'Spanish',
    'es-CL': 'Spanish',
    'es-CO': 'Spanish',
    'es-CR': 'Spanish',
    'es-CU': 'Spanish',
    'es-DO': 'Spanish',
    'es-EA': 'Spanish',
    'es-EC': 'Spanish',
    'es-ES': 'Spanish',
    'es-GQ': 'Spanish',
    'es-GT': 'Spanish',
    'es-HN': 'Spanish',
    'es-IC': 'Spanish',
    'es-MX': 'Spanish',
    'es-NI': 'Spanish',
    'es-PA': 'Spanish',
    'es-PE': 'Spanish',
    'es-PH': 'Spanish',
    'es-PR': 'Spanish',
    'es-PY': 'Spanish',
    'es-SV': 'Spanish',
    'es-US': 'Spanish',
    'es-UY': 'Spanish',
    'es-VE': 'Spanish',
    'et': 'Estonian',
    'et-EE': 'Estonian',
    'eu': 'Basque',
    'fa': 'Persian',
    'fa-AF': 'Persian',
    'fa-IR': 'Persian',
    'ff': 'Fulah',
    'ff-Adlm': 'Fulah',
    'ff-Adlm-BF': 'Fulah',
    'ff-Adlm-CM': 'Fulah',
    'ff-Adlm-GH': 'Fulah',
    'ff-Adlm-GM': 'Fulah',
    'ff-Adlm-GW': 'Fulah',
    'ff-Adlm-LR': 'Fulah',
    'ff-Adlm-MR': 'Fulah',
    'ff-Adlm-NE': 'Fulah',
    'ff-Adlm-NG': 'Fulah',
    'ff-Adlm-SL': 'Fulah',
    'ff-Adlm-SN': 'Fulah',
    'ff-Latn': 'Fulah',
    'ff-Latn-BF': 'Fulah',
    'ff-Latn-CM': 'Fulah',
    'ff-Latn-GH': 'Fulah',
    'ff-Latn-GM': 'Fulah',
    'ff-Latn-GN': 'Fulah',
    'ff-Latn-GW': 'Fulah',
    'ff-Latn-LR': 'Fulah',
    'ff-Latn-MR': 'Fulah',
    'ff-Latn-NE': 'Fulah',
    'ff-Latn-NG': 'Fulah',
    'ff-Latn-SL': 'Fulah',
    'fi': 'Finnish',
    'fi-FI': 'Finnish',
    'fil-PH': 'Filipino',
    'fo': 'Faroese',
    'fo-DK': 'Faroese',
    'fr': 'French',
    'fr-BE': 'French',
    'fr-BF': 'French',
    'fr-BI': 'French',
    'fr-BJ': 'French',
    'fr-BL': 'French',
    'fr-CA': 'French',
    'fr-CD': 'French',
    'fr-CF': 'French',
    'fr-CG': 'French',
    'fr-CH': 'French',
    'fr-CI': 'French',
    'fr-CM': 'French',
    'fr-DJ': 'French',
    'fr-DZ': 'French',
    'fr-FR': 'French',
    'fr-GA': 'French',
    'fr-GF': 'French',
    'fr-GN': 'French',
    'fr-GP': 'French',
    'fr-GQ': 'French',
    'fr-HT': 'French',
    'fr-KM': 'French',
    'fr-LU': 'French',
    'fr-MA': 'French',
    'fr-MC': 'French',
    'fr-MF': 'French',
    'fr-MG': 'French',
    'fr-ML': 'French',
    'fr-MQ': 'French',
    'fr-MR': 'French',
    'fr-MU': 'French',
    'fr-NC': 'French',
    'fr-NE': 'French',
    'fr-PF': 'French',
    'fr-PM': 'French',
    'fr-RE': 'French',
    'fr-RW': 'French',
    'fr-SC': 'French',
    'fr-SN': 'French',
    'fr-SY': 'French',
    'fr-TD': 'French',
    'fr-TG': 'French',
    'fr-TN': 'French',
    'fr-VU': 'French',
    'fr-WF': 'French',
    'fr-YT': 'French',
    'fy': 'Western Frisian',
    'ga': 'Irish',
    'ga-GB': 'Irish',
    'gd': 'Scottish Gaelic',
    'gl': 'Galician',
    'gn': 'Guarani',
    'gu': 'Gujarati',
    'gu-IN': 'Gujarati',
    'gv': 'Manx',
    'ha': 'Hausa',
    'ha-Arab': 'Hausa',
    'ha-Arab-SD': 'Hausa',
    'ha-GH': 'Hausa',
    'ha-NE': 'Hausa',
    'he': 'Hebrew',
    'he-IL': 'Hebrew',
    'hi': 'Hindi',
    'hi-IN': 'Hindi',
    'hi-Latn': 'Hindi',
    'hr': 'Croatian',
    'hr-BA': 'Croatian',
    'hr-HR': 'Croatian',
    'ht': 'Haitian',
    'hu': 'Hungarian',
    'hu-HU': 'Hungarian',
    'hy': 'Armenian',
    'ia': 'Interlingua',
    'id': 'Indonesian',
    'id-ID': 'Indonesian',
    'ie': 'Interlingue',
    'ig': 'Igbo',
    'ii': 'Sichuan Yi',
    'ik': 'Inupiaq',
    'io': 'Ido',
    'is': 'Icelandic',
    'it': 'Italian',
    'it-CH': 'Italian',
    'it-IT': 'Italian',
    'it-SM': 'Italian',
    'it-VA': 'Italian',
    'iu': 'Inuktitut',
    'iu-Latn': 'Inuktitut',
    'ja': 'Japanese',
    'ja-JP': 'Japanese',
    'jv': 'Javanese',
    'ka': 'Georgian',
    'ki': 'Kikuyu',
    'kk': 'Kazakh',
    'kk-Arab': 'Kazakh',
    'kk-Cyrl': 'Kazakh',
    'kk-KZ': 'Kazakh',
    'kl': 'Kalaallisut',
    'km': 'Central Khmer',
    'kn': 'Kannada',
    'kn-IN': 'Kannada',
    'ko': 'Korean',
    'ko-CN': 'Korean',
    'ko-KP': 'Korean',
    'ko-KR': 'Korean',
    'ks': 'Kashmiri',
    'ks-Arab': 'Kashmiri',
    'ks-Deva': 'Kashmiri',
    'ku': 'Kurdish',
    'kw': 'Cornish',
    'ky': 'Kyrgyz',
    'la': 'Latin',
    'lb': 'Luxembourgish',
    'lg': 'Ganda',
    'ln': 'Lingala',
    'ln-AO': 'Lingala',
    'ln-CF': 'Lingala',
    'ln-CG': 'Lingala',
    'lo': 'Lao',
    'lt': 'Lithuanian',
    'lt-LT': 'Lithuanian',
    'lu': 'Luba-Katanga',
    'lv': 'Latvian',
    'lv-LV': 'Latvian',
    'mg': 'Malagasy',
    'mi': 'Maori',
    'mk': 'Macedonian',
    'ml': 'Malayalam',
    'ml-IN': 'Malayalam',
    'mn': 'Mongolian',
    'mn-Mong': 'Mongolian',
    'mn-Mong-MN': 'Mongolian',
    'mr': 'Marathi',
    'mr-IN': 'Marathi',
    'ms': 'Malay',
    'ms-Arab': 'Malay',
    'ms-Arab-BN': 'Malay',
    'ms-BN': 'Malay',
    'ms-ID': 'Malay',
    'ms-SG': 'Malay',
    'mt': 'Maltese',
    'my': 'Burmese',
    'nb': 'Norwegian Bokmål',
    'nb-SJ': 'Norwegian Bokmål',
    'nd': 'North Ndebele',
    'ne': 'Nepali',
    'ne-IN': 'Nepali',
    'nl': 'Dutch',
    'nl-AW': 'Dutch',
    'nl-BE': 'Dutch',
    'nl-BQ': 'Dutch',
    'nl-CW': 'Dutch',
    'nl-NL': 'Dutch',
    'nl-SR': 'Dutch',
    'nl-SX': 'Dutch',
    'nn': 'Norwegian Nynorsk',
    'no': 'Norwegian',
    'no-NO': 'Norwegian',
    'nr': 'South Ndebele',
    'nv': 'Navajo',
    'ny': 'Chichewa',
    'oc': 'Occitan',
    'oc-ES': 'Occitan',
    'om': 'Oromo',
    'om-KE': 'Oromo',
    'or': 'Oriya',
    'os': 'Ossetian',
    'os-RU': 'Ossetian',
    'pa': 'Punjabi',
    'pa-IN': 'Punjabi',
    'pa-Arab': 'Punjabi',
    'pa-Guru': 'Punjabi',
    'pl': 'Polish',
    'pl-PL': 'Polish',
    'ps': 'Pashto',
    'ps-PK': 'Pashto',
    'pt': 'Portuguese',
    'pt-AO': 'Portuguese',
    'pt-BR': 'Portuguese',
    'pt-CH': 'Portuguese',
    'pt-CV': 'Portuguese',
    'pt-GQ': 'Portuguese',
    'pt-GW': 'Portuguese',
    'pt-LU': 'Portuguese',
    'pt-MO': 'Portuguese',
    'pt-MZ': 'Portuguese',
    'pt-PT': 'Portuguese',
    'pt-ST': 'Portuguese',
    'pt-TL': 'Portuguese',
    'qu': 'Quechua',
    'qu-BO': 'Quechua',
    'qu-EC': 'Quechua',
    'rm': 'Romansh',
    'rn': 'Rundi',
    'ro': 'Romanian',
    'ro-MD': 'Romanian',
    'ro-RO': 'Romanian',
    'ru': 'Russian',
    'ru-BY': 'Russian',
    'ru-KG': 'Russian',
    'ru-KZ': 'Russian',
    'ru-MD': 'Russian',
    'ru-RU': 'Russian',
    'ru-UA': 'Russian',
    'rw': 'Kinyarwanda',
    'sa': 'Sanskrit',
    'sc': 'Sardinian',
    'sd': 'Sindhi',
    'sd-Arab': 'Sindhi',
    'sd-Deva': 'Sindhi',
    'se': 'Northern Sami',
    'se-FI': 'Northern Sami',
    'se-SE': 'Northern Sami',
    'sg': 'Sango',
    'si': 'Sinhala',
    'sk': 'Slovak',
    'sk-SK': 'Slovak',
    'sl': 'Slovenian',
    'sl-SI': 'Slovenian',
    'sn': 'Shona',
    'so': 'Somali',
    'so-DJ': 'Somali',
    'so-ET': 'Somali',
    'so-KE': 'Somali',
    'sq': 'Albanian',
    'sq-MK': 'Albanian',
    'sq-XK': 'Albanian',
    'sr': 'Serbian',
    'sr-RS': 'Serbian',
    'sr-Cyrl': 'Serbian',
    'sr-Cyrl-BA': 'Serbian',
    'sr-Cyrl-ME': 'Serbian',
    'sr-Cyrl-XK': 'Serbian',
    'sr-Latn': 'Serbian',
    'sr-Latn-BA': 'Serbian',
    'sr-Latn-ME': 'Serbian',
    'sr-Latn-XK': 'Serbian',
    'ss': 'Swati',
    'ss-SZ': 'Swati',
    'st': 'Southern Sotho',
    'st-LS': 'Southern Sotho',
    'su': 'Sundanese',
    'su-Latn': 'Sundanese',
    'sv': 'Swedish',
    'sv-AX': 'Swedish',
    'sv-FI': 'Swedish',
    'sv-SE': 'Swedish',
    'sw': 'Swahili',
    'sw-CD': 'Swahili',
    'sw-KE': 'Swahili',
    'sw-TZ': 'Swahili',
    'sw-UG': 'Swahili',
    'ta': 'Tamil',
    'ta-IN': 'Tamil',
    'ta-LK': 'Tamil',
    'ta-MY': 'Tamil',
    'ta-SG': 'Tamil',
    'te': 'Telugu',
    'te-IN': 'Telugu',
    'tg': 'Tajik',
    'th': 'Thai',
    'th-TH': 'Thai',
    'ti': 'Tigrinya',
    'ti-ER': 'Tigrinya',
    'tk': 'Turkmen',
    'tl': 'Tagalog',
    'tn': 'Tswana',
    'tn-BW': 'Tswana',
    'to': 'Tonga',
    'tr': 'Turkish',
    'tr-CY': 'Turkish',
    'tr-TR': 'Turkish',
    'ts': 'Tsonga',
    'tt': 'Tatar',
    'ug': 'Uyghur',
    'uk': 'Ukrainian',
    'uk-UA': 'Ukrainian',
    'ur': 'Urdu',
    'ur-IN': 'Urdu',
    'ur-PK': 'Urdu',
    'uz': 'Uzbek',
    'uz-Arab': 'Uzbek',
    'uz-Cyrl': 'Uzbek',
    'uz-Latn': 'Uzbek',
    've': 'Venda',
    'vi': 'Vietnamese',
    'vi-VN': 'Vietnamese',
    'vo': 'Volapük',
    'wa': 'Walloon',
    'wo': 'Wolof',
    'xh': 'Xhosa',
    'yi': 'Yiddish',
    'yo': 'Yoruba',
    'yo-BJ': 'Yoruba',
    'za': 'Zhuang',
    'zh': 'Chinese',
    'zh-CH': 'Chinese',
    'zh-TW': 'Chinese',
    'zh-Hans': 'Chinese',
    'zh-Hans-HK': 'Chinese',
    'zh-Hans-MO': 'Chinese',
    'zh-Hans-MY': 'Chinese',
    'zh-Hans-SG': 'Chinese',
    'zh-Hant': 'Chinese',
    'zh-Hant-HK': 'Chinese',
    'zh-Hant-MO': 'Chinese',
    'zh-Hant-MY': 'Chinese',
    'zh-Latn': 'Chinese',
    'zu': 'Zulu',
    'zu-ZA': 'Zulu',
}

# Colors for more readable output
TEXT_COLOR_NONE   = '\x1b[0m'
TEXT_COLOR_BLUE   = '\x1b[0;34m'
TEXT_COLOR_CYAN   = '\x1b[0;36m'
TEXT_COLOR_GREEN  = '\x1b[0;32m'
TEXT_COLOR_ORANGE = '\x1b[0;33m'
TEXT_COLOR_PURPLE = '\x1b[0;35m'
TEXT_COLOR_RED    = '\x1b[0;31m'
def str_color_blue(s):
    if not COLOR_LOG_OUTPUT:
        return s
    return f'{TEXT_COLOR_BLUE}{s}{TEXT_COLOR_NONE}'
def str_color_cyan(s):
    if not COLOR_LOG_OUTPUT:
        return s
    return f'{TEXT_COLOR_CYAN}{s}{TEXT_COLOR_NONE}'
def str_color_green(s):
    if not COLOR_LOG_OUTPUT:
        return s
    return f'{TEXT_COLOR_GREEN}{s}{TEXT_COLOR_NONE}'
def str_color_orange(s):
    if not COLOR_LOG_OUTPUT:
        return s
    return f'{TEXT_COLOR_ORANGE}{s}{TEXT_COLOR_NONE}'
def str_color_purple(s):
    if not COLOR_LOG_OUTPUT:
        return s
    return f'{TEXT_COLOR_PURPLE}{s}{TEXT_COLOR_NONE}'
def str_color_red(s):
    if not COLOR_LOG_OUTPUT:
        return s
    return f'{TEXT_COLOR_RED}{s}{TEXT_COLOR_NONE}'


# =================== CONFIG ====================
ORIGINAL_LANGUAGE_FILE = 'en.json'
LANGUAGE_SHORT = 'el'
BASE_URL = 'http://192.168.1.123:1234/api'
API_KEY = 'sk-00000000000000000000000000000000'  # For example, an Open WebUI API key
MODEL_NAME = 'translategemma-4b-it'
TRANSLATOR = 'Sean Pesce'
TRANSLATOR_CONTACT = 'jcd@unatco.gov'
OVERWRITE = False  # Overwrite translated file when re-running the script
COLOR_LOG_OUTPUT = True
ALWAYS_SPLIT_ON_SPECIAL_DELIMS = False  # Force split and re-build strings that use "//(1.00,2.99)\\" and/or "{0}" delimiters (these often get mangled by the LLM)

# Parse first command-line argument to optionally override target language
if len(sys.argv) > 1 and sys.argv[1].strip() in LANGUAGES:
    LANGUAGE_SHORT = sys.argv[1].strip()
elif len(sys.argv) > 1:
    print(f'{str_color_orange("[WARNING]")} Invalid language code "{sys.argv[1].strip()}"; using "{LANGUAGE_SHORT}" instead', file=sys.stderr)
LANGUAGE = LANGUAGES[LANGUAGE_SHORT]
SOURCE_LANGUAGE_SHORT = 'en'  # Only change if source language dataset is non-English (not recommended - will likely break due to optimization code)
SOURCE_LANGUAGE = LANGUAGES[SOURCE_LANGUAGE_SHORT]
# Based on the official TranslateGemma prompt guidance, found here:
#   https://ollama.com/library/translategemma#:~:text=device%20they%20own.-,Prompt%20Guide,-Prompt%20Format
PROMPT = f'You are a professional {SOURCE_LANGUAGE} ({SOURCE_LANGUAGE_SHORT}) to {LANGUAGE} ({LANGUAGE_SHORT}) translator. Your goal is to accurately convey the meaning and nuances of the original {SOURCE_LANGUAGE} text while adhering to {LANGUAGE} grammar, vocabulary, and cultural sensitivities.\n' +\
         f'Produce only the {LANGUAGE} translation, without any additional explanations or commentary. Please translate the following {SOURCE_LANGUAGE} text into {LANGUAGE}:\n\n'
INDENT = '  '  # Indentation for formatted JSON data
RETRIES = 10
RETRY_DELAY = 5  # seconds
ENCODING = 'utf8'

RIGHT_TO_LEFT=False
if LANGUAGE.lower() in {'arabic','fulah','hebrew','kurdish','pashto','persian','punjabi','urdu',}:
    RIGHT_TO_LEFT = True
# =================== /CONFIG ===================

TOTAL_STRINGS = 0
TOTAL_STRINGS_TRANSLATED = 0
START_TIME = time.time()
END_TIME = None

# Cache strings that were already translated to avoid unnecessary compute and drastically decrease completion time
TRANSLATION_CACHE = dict()

# Strings that should not be translated
SKIP_TRANSLATION = {
    'English',
    'Français',
    'Italiano',
    'Deutsch',
    'Español (España)',
    'русский',
    'Polski',
    'Español (América Latina)',
    'Português (Brasil)',
    'Japanese',
    
    'Facebook',
    'YouTube',
}
SKIP_TRANSLATION_LOWER = None
# Make sure we translate the source language
dont_skip = None
for s in SKIP_TRANSLATION:
    if SOURCE_LANGUAGE.lower() == s.lower():
        dont_skip = s
        break
if dont_skip:
    SKIP_TRANSLATION.discard(dont_skip)



# List of in-game passwords that should not be modified
#   https://deusex.fandom.com/wiki/Passwords_(DXMD)
#   https://deusex.fandom.com/wiki/A_Criminal_Past_walkthrough#Keycodes_&_Passwords
IN_GAME_PWS = {
    '0724',
    '1114',
    '1233',
    '1337',
    '1423',
    '1969',
    '1977',
    '1980',
    '1FreeWish',
    '2027',
    '2245',
    '2297',
    '3014',
    '3322',
    '3469',
    '3497',
    '3499',
    '3546',
    '3579',
    '3624',
    '3648',
    '3677',
    '3698',
    '3791',
    '4110',
    '4286',
    '4469',
    '4679',
    '4992',
    '4ndr3wSHU',
    '5123',
    '5359',
    '5439',
    '5896KLMO1BN',
    '6014',
    '6435',
    '6484',
    '6632',
    '6645',
    '6701',
    '6912',
    '7925',
    '7946',
    '8236',
    '8642',
    '8821',
    '8822',
    '8824',
    '9110',
    '9316',
    '9357',
    '9365',
    '9378',
    '9458',
    '9558',
    '9643',
    '9684',
    '9786',
    '9824',
    '9835',
    '9991',
    '9992',
    'AKLVD6681',
    'AUZ1LU51AT1',
    'Alli300',
    'Almeisan',
    'AnaStefanJelena',
    'Aphasia5689',
    'AugBuster001',
    'AugBuster002',
    'AugBuster003',
    'B0TT0PP5',
    'BBput5841',
    'BaconChipsDonut',
    'BertiLLon1870',
    'Bla14f5ku6',
    'BuiLtForUs',
    'CL4NKN3T1ON',
    'CatBathTime',
    'Cha12f5g9e',
    'CharadeYouAre',
    'Charlieclaw',
    'Christelle',
    'Clod04sfd8s',
    'DELLAROCCA',
    'DevilToldMe',
    'DobraNoc',
    'DriedGrapes',
    'E4E5QH5',
    'ELEFSIS',
    'ELVIRA88',
    'Egypt2030',
    'ElectricSpirit',
    'FURVENT6',
    'FaithInU',
    'FrakkingUpTheA',
    'GBNM45KLP53',
    'HYPERION',
    'HorseKing',
    'ImCarcosa',
    'JJKPOXZ774',
    'JLAROUCHE',
    'KEPLER432B',
    'KLMVP9951',
    'LEPTONS',
    'LKMN01259KNML',
    'LambScream',
    'MACHINEGUN',
    'MASKEDCYBORG',
    'MUSTBREAKU',
    'May2020',
    'McCr49f0eF',
    'Messier43',
    'Messier45',
    'Middleearth',
    'MoonShine',
    'N4RDIN3M0R4N0',
    'NEBUKED',
    'NMKKHJ223',
    'NOFATE',
    'NOMORE',
    'NanoSpika',
    'Neuronex1000',
    #'Neuropozyne',
    'OCTAVARIUM',
    'OL519xanadu',
    'OVCHARKA',
    'Origami1970',
    'Ouroboros',
    'P34T45WF',
    'P3WP3WS',
    'PALSEC4654TI',
    'PALSEC6579AT',
    'PunchRocket',
    'Raptor',
    'RedEyePatched',
    'S3CUR3SYST3M',
    'ST33LB3AM5',
    'STRMSRG',
    #'SUNGLASSES',
    'SWD5469',
    'Sharp007',
    'Slaw',
    'SoulSlave',
    #'Symphony',
    'T1000',
    #'TBD',
    'TheFinalCut',
    'ThemBones',
    'TrustNo1',
    'Ubik',
    'VCORPP03L',
    #'VENUS',
    'VIPsec487Tv',
    'VODKACHRONIC69',
    'VREMENA77',
    'Wiedzmin',
    'YABLOKA346',
    'absolution',
    'andrespoutine',
    'antares',
    'arcturus',
    #'ascension',
    'brtvz5524',
    'civilizat10N',
    'clemenza',
    'copyreport00',
    'd3f4gh6',
    #'dawn',
    'devotchka',
    'dormata',
    'dyc2p0201',
    'ebert',
    'everforward',
    'fireworks',
    'goldenage',
    'heartofdarkness',
    'highdata001',
    'hjtrv8865',
    'intermission',
    'kollerdark',
    'kristab',
    'krodine',
    'legacy2025',
    'linox2014',
    'lowzone16',
    'masaaki',
    'mcsteamy',
    'n3wsm4ker',
    'nardinemorano',
    'nmkkhj223',
    'norestforus',
    'oL519Xanadu',
    'otarminator',
    'ou812',
    'owningthedevil',
    'paulwestlind',
    'pozy45',
    'praisethesun',
    #'remember',
    'respawn777',
    'rosebud',
    'safeandsecure22',
    'samithedog',
    'saveprivateroy',
    'scheherezade',
    'secretpocketary',
    'starlight',
    'stormsurge',
    'strengthinunity',
    'thecause',
    'thevoidwhichbinds',
    'tornado',
    'tuscany2023',
    'tycoon999',
    'vedelem',
    'vincam',
    'vm451',
    'withstrangeaeons',
}
SKIP_TRANSLATION.update(IN_GAME_PWS)


# Textlists to skip (e.g., if they only contain passwords)
SKIP_TEXTLISTS = {
    15727827938747724,
    21394524206646751,
    32952490151201634,
    33367695002050062,
    52516518959670934,
    5687818836357650,
    57838673402989992,
    62575541654534117,
    63850479627289443,
    70969920398094849,
}

SPECIAL_DELIMITER_REGEX = r'((?://\s*\([0-9\.,]+\)\s*\\\\)|(?:\s*\{[0-9]\}\s*))'
SPECIAL_DELIMITER_AND_PW_REGEX = SPECIAL_DELIMITER_REGEX[1:-1]
for pw in IN_GAME_PWS:
    if not pw:
        continue
    SPECIAL_DELIMITER_AND_PW_REGEX += '|(?:' + pw + ')'
SPECIAL_DELIMITER_AND_PW_REGEX = '(' + SPECIAL_DELIMITER_AND_PW_REGEX + ')'
SPECIAL_DELIMITER_AND_PW_REGEX = re.compile(SPECIAL_DELIMITER_AND_PW_REGEX, flags=re.IGNORECASE)


def convert_rtl_string(s):
    # The DXMD game engine doesn't support right-to-left, so we have to manually craft it
    # Reverse string except for numeric data
    tokens = re.findall(r'\d+|\D', s)  # Multi-digit number, or any non-digit character
    tokens.reverse()
    s = ''.join(tokens)
    
    # Flip position-dependent characters
    reverse_chars = {
        '[': ']',
        ']': '[',
        '(': ')',
        ')': '(',
        '<': '>',
        '>': '<',
        '«': '»',
        '»': '«',
        #'“': '”',
        #'”': '“',
        '“': '"',  # Convert to position-agnostic double quote instead
        '”': '"',  # Convert to position-agnostic double quote instead
        '„': '"',  # Convert to position-agnostic double quote instead
        #'‘': '’',
        #'’': '‘',
        '‘': '\'',  # Convert to position-agnostic apostrophe instead
        '’': '\'',  # Convert to position-agnostic apostrophe instead
        #'“': '„',
        #'„': '“'
    }
    #if '„' in s:
    #    reverse_chars['“'] = '„'
    s = ''.join(reverse_chars.get(ch, ch) for ch in s)
    return s


def translate_string(s, ignore_rtl=False):
    global DATA_TRANSLATED
    global TOTAL_STRINGS
    global TOTAL_STRINGS_TRANSLATED
    global SKIP_TRANSLATION_LOWER
    if not SKIP_TRANSLATION_LOWER:
      # Initialize case-insensitive string set
      SKIP_TRANSLATION_LOWER = set()
      for skip_s in SKIP_TRANSLATION:
          SKIP_TRANSLATION_LOWER.add(skip_s.lower().strip())
    TOTAL_STRINGS += 1
    if (not s) or (not s.strip()):
        return s
    if s.strip().lower() == SOURCE_LANGUAGE.strip().lower():
        # Change the source language to the target language so that it is displayed correctly in the game menus
        s = LANGUAGE
    # Check if this string was already translated
    if s.strip() in TRANSLATION_CACHE:
        return TRANSLATION_CACHE[s.strip()]
    # Check if the string should be skipped
    if s.strip().lower() in SKIP_TRANSLATION_LOWER:
        TRANSLATION_CACHE[s] = s
        return s
    # Check if the string has any alphabetic characters (@NOTE: This will probably break things if the source data set is non-English)
    if not any(c.isalpha() for c in s):
        # The string is only numbers and/or special characters, so no need to translate it
        #TRANSLATION_CACHE[s] = s  # Don't cache (e.g., delimiters might have varied whitespace)
        return s
    s = s.strip()
    client = OpenAI(
        base_url=BASE_URL,
        api_key=API_KEY
    )

    # Check for strings wrapped in special characters (e.g., "[cough]")
    prefix = ''
    suffix = ''
    wrap_chars = '[]()-_'#{}
    # if s[0] in wrap_chars:
    #     prefix = s[0]
    #     s = s[1:]
    # if s[-1] in wrap_chars:
    #     suffix = s[-1]
    #     s = s[:-1]
    if (s[0] in wrap_chars) and (s[-1] in wrap_chars):
        prefix = s[0]
        s = s[1:]
        suffix = s[-1]
        s = s[:-1]

    for i in range(0, RETRIES):
        try:
            response = client.chat.completions.create(
                model=MODEL_NAME,
                messages=[
                    {'role': 'system', 'content': PROMPT,},
                    {'role': 'user', 'content': s}
                ],
                temperature=0.0
            )
            translated_item = response.choices[0].message.content.strip()
            #translated_item = translated_item.replace('\\\n','').strip()  # Artifact that sometimes occurs on strings with newlines (?)
            if prefix:
                s = prefix + s
                translated_item = prefix + translated_item
            if suffix:
                s = s + suffix
                translated_item = translated_item + suffix
            # Don't reverse RTL strings that aren't rendered in the game engine
            splash_screen_strings = {
                'Play Deus Ex: Mankind Divided™',
                'Play Deus Ex: Breach™',
                'Website',
                'Facebook',
                'YouTube',
                'Technical Support',
            }
            if s in splash_screen_strings or (s == translated_item):
                ignore_rtl = True
            if (not ignore_rtl) and RIGHT_TO_LEFT:
                translated_item = convert_rtl_string(translated_item)
            print(f'{str_color_purple(json.dumps(s, ensure_ascii=False))}  ->  {str_color_cyan(json.dumps(translated_item, ensure_ascii=False))}', file=sys.stderr)
            if translated_item and (s not in TRANSLATION_CACHE):
                TRANSLATION_CACHE[s] = translated_item
            TOTAL_STRINGS_TRANSLATED += 1
            return translated_item
        except Exception as err:
            if i < (RETRIES-1):
                print(f'{str_color_orange("[WARNING]")} (Attempt {i+1}/{RETRIES}) {err} When attempting to translate string: {json.dumps(s)}', file=sys.stderr)
                time.sleep(RETRY_DELAY)
            else:
                #raise err
                pass
    return None


def split_str_by_special_delimiters(s):
    # Some textlist/subtitles have timestamp delimiters like: //(0.000)\\ or //(0.000,1.111)\\
    # (These often get mangled by the LLM)
    # And some textlist entries have format-string delimiters like: {0} or {6}
    # (These can get mangled by the LLM)
    global SPECIAL_DELIMITER_AND_PW_REGEX
    pattern = SPECIAL_DELIMITER_AND_PW_REGEX #r'((?://\s*\([0-9\.,]+\)\s*\\\\)|(?:\s*\{[0-9]\}\s*))'
    # re.split returns a list, splitting on the pattern
    parts = re.split(pattern, s)
    # Remove empty strings resulting from leading/trailing delimiters
    parts = [part for part in parts if part]
    #print(str_color_red(json.dumps(parts, indent='  ', ensure_ascii=False)), file=sys.stderr)
    return parts


def translate_textlists():
    global TOTAL_STRINGS
    DATA_TRANSLATED['textlists'] = list()
    textlists_orig = DATA_ORIGINAL['textlists']
    i = 0
    for textlist in textlists_orig:
        i += 1
        # if i >= 2:
        #     # Finish early for debugging purposes
        #     break
        print(f'{str_color_green("[INFO]")} ({i}/{len(textlists_orig)}) Translating textlist', file=sys.stderr)
        tl = dict()
        tl_id = textlist.get('id', None)
        if tl_id in SKIP_TEXTLISTS:
            print(f'{str_color_green("[INFO]")} Skipping textlist with ID {tl_id}', file=sys.stderr)
            TOTAL_STRINGS += len(textlist.get('content', list()))
            continue
        for k in textlist:
            if k != 'content':
                tl[k] = textlist[k]
                continue
        tl[k] = list()
        for data in textlist['content']:
            translated_item = dict()
            for kk in data:
                if kk != 'string':
                    translated_item[kk] = data[kk]
                    continue
                
                translated_item[kk] = ''
                split_str = split_str_by_special_delimiters(data[kk])
                if ALWAYS_SPLIT_ON_SPECIAL_DELIMS:
                    # Split and re-build strings that use "//(1.00,2.99)\\" or "{0}" delimiters (these often get mangled by the LLM)
                    for str_part in split_str:
                        translated_item[kk] += translate_string(str_part)
                else:
                    translated_item[kk] = translate_string(data[kk])
                    # Check that no delimiters were dropped. If they were, we force a re-translation using the split method
                    if len(split_str) > 1:
                        split_translated = split_str_by_special_delimiters(translated_item[kk])
                        if len(split_translated) != len(split_str):
                            print(f'{str_color_orange("[WARNING]")} Special string delimiter was dropped during translation. Re-translating using split string method...', file=sys.stderr)
                            translated_item[kk] = ''
                            for str_part in split_str:
                                translated_item[kk] += translate_string(str_part)
            tl[k].append(translated_item)
        DATA_TRANSLATED['textlists'].append(tl)
    return DATA_TRANSLATED['textlists']



def translate_subtitles():
    DATA_TRANSLATED['subtitles'] = list()
    subtitles_orig = DATA_ORIGINAL['subtitles']
    i = 0
    for subtitle in subtitles_orig:
        i += 1
        # if i >= 2:
        #     # Finish early for debugging purposes
        #     break
        print(f'{str_color_green("[INFO]")} ({i}/{len(subtitles_orig)}) Translating subtitles', file=sys.stderr)
        ss = dict()
        for k in subtitle:
            if k != 'content':
                ss[k] = subtitle[k]
                continue
        ss[k] = list()
        j = 0
        for data in subtitle['content']:
            j += 1
            # if j >= 2:
            #     # Finish early for debugging purposes
            #     break
            translated_subs = dict()
            for kk in data:
                if kk != 'subs':
                    translated_subs[kk] = data[kk]
                    continue
                translated_subs['subs'] = list()
                for subs_set in data['subs']:
                    translated_subtitle = dict()
                    for kkkk in subs_set:
                        if kkkk != 'string':
                            translated_subtitle[kkkk] = subs_set[kkkk]
                            continue
                        
                        translated_subtitle[kkkk] = ''
                        split_str = split_str_by_special_delimiters(subs_set[kkkk])
                        if ALWAYS_SPLIT_ON_SPECIAL_DELIMS:
                            # Split and re-build strings that use "//(1.00,2.99)\\" or "{0}" delimiters (these often get mangled by the LLM)
                            for subtitle_part in split_str:
                                translated_subtitle[kkkk] += translate_string(subtitle_part)
                        else:
                            translated_subtitle[kkkk] = translate_string(subs_set[kkkk])
                            # Check that no delimiters were dropped. If they were, we force a re-translation using the split method
                            if len(split_str) > 1:
                                split_translated = split_str_by_special_delimiters(translated_subtitle[kkkk])
                                if len(split_translated) != len(split_str):
                                    print(f'{str_color_orange("[WARNING]")} Timestamp delimiter was dropped during translation. Re-translating using split string method...', file=sys.stderr)
                                    translated_subtitle[kkkk] = ''
                                    for str_part in split_str:
                                        translated_subtitle[kkkk] += translate_string(str_part)
                        
                    translated_subs[kk].append(translated_subtitle)
                    
                #print(f'{json.dumps(data)}',file=sys.stderr)
            ss[k].append(translated_subs)
        DATA_TRANSLATED['subtitles'].append(ss)
    return DATA_TRANSLATED['subtitles']


def format_timedelta(seconds):
    """Pretty-prints a time difference as 'X hours, Y minutes, and Z seconds'."""
    msg = ''
    try:
        td = datetime.timedelta(seconds=seconds)
        total_seconds = int(td.total_seconds())
        minutes, seconds = divmod(total_seconds, 60)
        hours, minutes = divmod(minutes, 60)
        msg = f'{hours} hour{"" if hours == 1 else "s"}, {minutes} minute{"" if minutes == 1 else "s"}, and {seconds} second{"" if seconds == 1 else "s"}'
    except Exception as err:
        print(f'{str_color_orange("[WARNING]")} Failed to format time: {err}', file=sys.stderr)
        msg = f'{int(seconds)} seconds'
    return msg



print(f'{str_color_green("[INFO]")} Loading {SOURCE_LANGUAGE} data from {ORIGINAL_LANGUAGE_FILE}', file=sys.stderr)
DATA_ORIGINAL = ''
with open(ORIGINAL_LANGUAGE_FILE, 'r', encoding=ENCODING) as f:
    DATA_ORIGINAL = f.read()
DATA_ORIGINAL = json.loads(DATA_ORIGINAL)
#print(DATA_ORIGINAL.keys(), file=sys.stderr)

print(f'{str_color_green("[INFO]")} Translating: {str_color_purple(SOURCE_LANGUAGE)}  ->  {str_color_cyan(LANGUAGE)}', file=sys.stderr)
DATA_TRANSLATED = dict()
for k in DATA_ORIGINAL:
    if k == 'textlists':
        DATA_TRANSLATED[k] = translate_textlists()
    elif k == 'subtitles':
        DATA_TRANSLATED[k] = translate_subtitles()
    elif k == 'translated_by':
        DATA_TRANSLATED[k] = f'{TRANSLATOR} with {MODEL_NAME}'
    elif k == 'translator_contact':
        DATA_TRANSLATED[k] = TRANSLATOR_CONTACT
    elif k == 'language':
        DATA_TRANSLATED[k] = LANGUAGE_SHORT
    elif k == 'language_long':
        DATA_TRANSLATED[k] = LANGUAGE
    elif k == 'encoding':
        DATA_TRANSLATED[k] = ENCODING
    elif k == 'dev_messages':
        DATA_TRANSLATED[k] = dict()
        for kk in DATA_ORIGINAL[k]:
            DATA_TRANSLATED[k][kk] = translate_string(DATA_ORIGINAL[k][kk], ignore_rtl=True)
    else:
        DATA_TRANSLATED[k] = DATA_ORIGINAL[k]

END_TIME = time.time()
print(f'{str_color_green("[INFO]")} Done. Processed {TOTAL_STRINGS} strings ({TOTAL_STRINGS_TRANSLATED} translated by LLM).', file=sys.stderr)
print(f'{str_color_green("[INFO]")} Time taken: {format_timedelta(END_TIME-START_TIME)}', file=sys.stderr)


out_fname_base = f'{LANGUAGE_SHORT}.json'
out_fname = out_fname_base
i = 0
while (not OVERWRITE) and os.path.exists(out_fname):
    i += 1
    out_fname = f'{out_fname_base}.{i}'
print(f'{str_color_green("[INFO]")} Writing translated data to file: {os.path.abspath(out_fname)}', file=sys.stderr)
with open(out_fname, 'w', encoding=ENCODING, newline='\n') as f:
    f.write(json.dumps(DATA_TRANSLATED, indent=INDENT, ensure_ascii=False))


if __name__ == '__main__':
    pass

