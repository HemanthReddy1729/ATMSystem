#include<WiFi.h>
#include<WiFiClientSecure.h>
#include<UniversalTelegramBot.h>
#include<ArduinoJson.h>
#include<EEPROM.h>

#define token "2102742798:AAFeLwvwl0RgmlARERp4RGNib_tAgr7N_hI"
const char* ssid="HTNAMEH"; //Wifi Name
const char* password="Hemanth@2002"; //Wifi Password

WiFiClientSecure client;
UniversalTelegramBot atm_bot(token,client);


//n is no of bits to read
//n=8 for OTP and n=4 for denomination value
int touch_pin_read(int n)
{
  //To read the number entered by user by pins
  //Example:01011101 (93)
  //Array to store binary input
  int arr[n]={0};
  //Pin 13-1 bit
  //Pin 14-0 bit
  int start=0;
  Serial.println();
  while(start<n)
  {
    if(touchRead(13)<40)
    {
      arr[start]=1;
      start++;
      Serial.print(1);
    }
    else if(touchRead(14)<40)
    {
      arr[start]=0;
      start++;
      Serial.print(0);
    }
    delay(500);
  }
  int enteredValue=0;
  for(int i=0;i<n;i++)
  {
    enteredValue=2*enteredValue+(arr[i]);
  }
  Serial.println();
  Serial.print("Value Entered : ");
  Serial.println(enteredValue);
  return(enteredValue);
}


int withdraw()
{
  int flag = 0;
  Serial.println("Enter No of 2000 Notes:");
  int val1 = touch_pin_read(4);
  Serial.println("Enter No of 1000 Notes:");
  int val2 = touch_pin_read(4);
  Serial.println("Enter No of 500 Notes:");
  int val3 = touch_pin_read(4);
  if((val1<=EEPROM.read(0))&&(val2<=EEPROM.read(1))&&(val3<=EEPROM.read(2)))
  {
    flag=(val1*2000)+(val2*1000)+(val3*500);
    //Updating denoinations
    EEPROM.write(0,EEPROM.read(0)-val1); 
    EEPROM.write(1,EEPROM.read(1)-val2); 
    EEPROM.write(2,EEPROM.read(2)-val3); 
    EEPROM.commit();
  }
  else
  {
    flag=0;
  }
  return flag;
}

int loggedin=0;

void botResponse(int msg_count)
{
  int i;
  for(i=0;i<msg_count;i++)
  {
    String userID =String(atm_bot.messages[i].chat_id);
    String message = atm_bot.messages[i].text;
    String user_name = atm_bot.messages[i].from_name;
    Serial.println("Message from "+user_name+" -- "+message);
    
    if(message=="/start")
    {
      String welcome = "Welcome "+user_name+"\n";
      welcome+="/login to authenticate with server\n";
      welcome+="/withdraw to withdraw amount\n";
      welcome+="/balance to get current balance\n";
      atm_bot.sendMessage(userID,welcome,"");
    }
    
    if(message=="/login")
    { 
      //command to send OTP
      int OTP = random(0,100);
      String text = "Use this OTP "+String(OTP)+" to login to your account.";
      atm_bot.sendMessage(userID,text,"");
      Serial.println("Enter the OTP sent to your telegram :");
      int enteredOTP=touch_pin_read(8);
      if(enteredOTP==OTP)
      {
        loggedin=1;
        String success="Login Successful\n";
        success+="You can now continue "+user_name;
        atm_bot.sendMessage(userID,success,"");
      }
      else
      {
        atm_bot.sendMessage(userID,"The entered OTP is incorrect.","");
      }
    }
    
    if(message=="/withdraw")
    { 
      //withdraw command to draw the money
      if(loggedin==1)
      {
        int amount=withdraw();
        if(amount)
        {
          String text="Withdawn money : "+String(amount)+"\n";
          int balance=(2000*EEPROM.read(0))+(1000*EEPROM.read(1))+(500*EEPROM.read(2));
          text=text+"Balance remainning : "+String(balance);
          atm_bot.sendMessage(userID,text,"");
        }
        else
        {
          atm_bot.sendMessage(userID,"NO SUFFICIENT BALANCE","");
        }
      }
      else
      {
        String unauthorized="Unauthorized!! Login Required\n";
        unauthorized+="/login to login";
        atm_bot.sendMessage(userID,unauthorized,"");
      }
    }
    
    if(message=="/balance")
    { 
      if(loggedin==1)
      {
        int balance=(2000*EEPROM.read(0))+(1000*EEPROM.read(1))+(500*EEPROM.read(2));
        atm_bot.sendMessage(userID,"Balance : "+String(balance),"");
      }
      else
      {
        String unauthorized="Unauthorized!! Login Required\n";
        unauthorized+="/login to login";
        atm_bot.sendMessage(userID,unauthorized,"");
      }
    }
  }
}


void setup()
{
  Serial.begin(115200); // baud rate= 115200 bits/sec
  pinMode(13,INPUT); // bit1 , denomination 2000
  pinMode(14,INPUT); // bit0 , denomination 1000
  pinMode(4,INPUT); // denomination 500
  EEPROM.begin(3); //EEPROM with size 3 to store balance of respective dominations
  EEPROM.write(0,5); // 5 2000's
  EEPROM.write(1,10); //10 1000's
  EEPROM.write(2,10); //10 500's
  EEPROM.commit();

  client.setInsecure();
  WiFi.mode(WIFI_STA); //Station Mode
  WiFi.disconnect(); //Disconnect if connected already
  delay(100); //A delay for sucessfull disconnection
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.println("\n  WiFi connected");
}


void loop() {
  //Checkin whether ESP32 is connected to WiFi
  if (WiFi.status()==WL_CONNECTED)  
  {
    int msg_count = atm_bot.getUpdates(atm_bot.last_message_received + 1);//msg_count is no of new messages
    while(msg_count) 
    { 
      //Run until the msg_count is not zero
      botResponse(msg_count);
      msg_count = atm_bot.getUpdates(atm_bot.last_message_received + 1);
    }
    delay(500); // delay of 0.5 seconds
  }
}
