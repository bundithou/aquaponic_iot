# -*- coding: utf-8 -*-
"""
Created on Wed Sep 11 15:07:10 2019

@author: bundi
"""

"""Install the following requirements:
    dialogflow        0.5.1
    google-api-core   1.4.1
"""
import config

from google.api_core.exceptions import InvalidArgument
from google.oauth2 import service_account

from linebot.models import (
    MessageEvent, TextMessage, TextSendMessage,ImageSendMessage,LocationSendMessage,TemplateSendMessage,FlexSendMessage,CarouselTemplate,CarouselColumn
)

import schedule_job
import datacontrol


text_to_be_analyzed         = "Hi!"


def detect_intent(text = text_to_be_analyzed):
    sent = ""
    if text == "hi":
        sent = TextSendMessage(text="No")
    
    if text == "summary":
        sent = TextSendMessage(text = schedule_job.summary())

    if text == "todolist":
        sent = TextSendMessage(text = schedule_job.week_job())
    
    if text == "now":
        sent = TextSendMessage(text = schedule_job.now())

    if text == "report":
        sent = TextSendMessage(text = schedule_job.report())
    
    if text == "count today":
        sent = TextSendMessage(text = datacontrol.counttoday())
    
    if text == "count all":
        sent = TextSendMessage(text = datacontrol.countall())
    print("Query text:" + text)
    return sent
    
