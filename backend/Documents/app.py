
from flask import Flask, request, abort
import config
import intent_matching
import schedule_job
from linebot import (
    LineBotApi, WebhookHandler
)
from linebot.exceptions import (
    InvalidSignatureError
)
from linebot.models import (
    MessageEvent, TextMessage, TextSendMessage, BeaconEvent, LocationSendMessage
)

app = Flask(__name__)

line_bot_api = LineBotApi(config.LINE_CHANNEL_ACCESS_TOKEN)
handler = WebhookHandler(config.LINE_CHANNEL_SECRET)

@app.route("/callback", methods=['POST'])
def callback():
    # get X-Line-Signature header value
    signature = request.headers['X-Line-Signature']

    # get request body as text
    body = request.get_data(as_text=True)
    app.logger.info("Request body: " + body)
    print(body)
    # handle webhook body
    try:
        handler.handle(body, signature)
    except InvalidSignatureError:
        abort(400)

    return 'OK'


@handler.add(MessageEvent, message=TextMessage)
def handle_message(event):
    '''
    handle message
    by type, or text intent
    '''
    if("@bot" in event.message.text):
        id = ""
        if str(event.source.type) == 'user':
            id = str(event.source).split(",")[1].split(":")[1].replace("\"","").replace("}","")
        else:
            id = str(event.source).split(",")[0].split(":")[1].replace("\"","").replace("{","")
            
        res = intent_matching.detect_intent(event.message.text.replace("@bot ",""))
        
        
        line_bot_api.reply_message(
            event.reply_token,
            res
            # TextSendMessage(text=res.query_result.intent.display_name)
            #TextSendMessage(text=res.query_result.fulfillment_text)
        )
        


to = "Ca1b93ecaa5c68d54db094c98523610bd"
@app.route("/cronjob/min", methods=['POST'])
def cronjob_min():
    str_msg = schedule_job.min_job()
    if str_msg == "":
        return 'OK'
    line_bot_api.push_message(to, TextSendMessage(text=str_msg))
    return 'OK'

@app.route("/cronjob/week", methods=['POST'])
def cronjob_week():
    str_msg = schedule_job.week_job()
    line_bot_api.push_message(to, TextSendMessage(text=str_msg))
    return 'OK'

@app.route("/cronjob/day", methods=['POST'])
def cronjob_summary():
    str_msg = schedule_job.summary()
    line_bot_api.push_message(to, TextSendMessage(text=str_msg))
    return 'OK'

@app.route("/waterpump", methods=['GET'])
def waterpump():
    str_msg = "water pump is not working"
    line_bot_api.push_message(to, TextSendMessage(text=str_msg))
    return 'OK'

if __name__ == "__main__":
    app.run()
