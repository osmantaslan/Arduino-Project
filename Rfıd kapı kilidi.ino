#include <EEPROM.h> 
#include <SPI.h>                    // SPI haberleşme kütüphanesi
#include <MFRC522.h>              // Kart kütüphanesi
#include <Wire.h>                 // Wire kütüphanesi
#include <LiquidCrystal_I2C.h>    // LCD için swri haberleşme kütüphanesi
#include <Keypad.h>               //Tuş takımı için kütüphane
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

LiquidCrystal_I2C lcd(0x3F, 16, 2); // LCD ekran için "lcd" isimli nesne üretildi, buradaki0x3F hex rakamını i2c_scanner programınıyükleyip seri ekrandan okuyoruz
                                    // Bazen farklı rakam okunabilir 0x27 gibi , orada ne okuyorsak onu yazacağız buraya.
                                    
MFRC522 mfrc522(10,9);              // kart okuyucu için nesne üretildi mfrc522 isimli, 10,9 ile haberleşiyor

char sifre[4]; //= {'1', '2', '3', '4'}; //ilk açılışta konulan şifre.
char sifre_t[4];                    //geçici olarak, girilen şifreyi tutar.
char y_sifre[4]={'1','9','2','3'}; // mevcut şifreeyi değiştirmek için kullanılan yönetici şifresi.
byte sk;
int a=0;
int sifre_sayac=0; // Şifre değiştirmek için 3 defa yıldıza basılacak onu sayıyor
const byte satir=4;
const byte sutun=3;
char alinan_tus=0; // Tuş takımından alınan tuşun kaydedildiği karakter değişkeni
char tus_takimi [satir] [sutun]={
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
  };

  byte satir_pinleri[satir]={6,5,4,A0}; // tuşların bağlandığı satırlar
  byte sutun_pinleri[sutun]={A1,A2,A3}; // tuşların bağlandığı sütünlar

  Keypad tus =Keypad (makeKeymap(tus_takimi),satir_pinleri, sutun_pinleri, satir, sutun); // Tuş takımı için nesne oluşturma ve tanımlamalar yapılıyor.

    // NOT:  kartsayisi değişkeni ve kartlar dizisi değişkeni aynı değerde olup, 4ün katı olmalılar.Tanımladığımız her kartın 4 adet numarası vardır.
    // Bu dört adet numara seri ekrandan okunup "kartlar" dizisine kaydedilmelidir. Kayıttan sonra dizi uzunluğunu ve "kartsayisi" değişkenini güncelle.
    // "Kartlar" dizisinde kaç adet rakam varsa dizi uzunluğu ve "kartsayisi" değişkeni aynı sayıda olmalıdır. Örneğin 2 adetkart tanımlayacak isek
    // "kartlar[8]" ve "kartsayisi=8" olmalıdır.Çünki her kartın 4 adet rakamdan oluşan bir şifresi vardır.
    
    int kartsayisi=16;
    int kartlar[16]={215,125,233,5,167,247,69,0,103,153,114,0,147,229,164,196};
    
boolean izin[4]; // kart kontrolünde kullanılan değişken dizi
boolean onay;   // kartın doğrulunu kontrol eden onay

int yled=7; // şifre yada kart doğru ise yanacak ve kapı açma rölesini aktif edecek
int buzzer=8; // uyarı sesi için kullanıan buzzer
int ei;

//----------------Parmak izi okuyucu tanımlamaları--------------------------
  int p=4 ; // kaç kişinin parmak izi tanımlandı ise p değişkeni içeriği o olmalı
  int parmak_izi_kisi_sayisi[4]; // dizi uzunluğu p ile aynı olmalı
  int alinan_parmakizi;
 

void setup() {
    Serial.begin(9600); 
  
  delay(1000);

  for(int i=0;i<p;i++)
  {
    parmak_izi_kisi_sayisi[i]=(i+1);
  }
 


  if(EEPROM.read(6)==255)
  {
  EEPROM.write(6,1);
  EEPROM.write(3,'4');
  EEPROM.write(2,'3');
  EEPROM.write(1,'2');
  EEPROM.write(0,'1'); 
 }

    for(int z=0; z<4; z++)    // eepromdan şifre okunuyor
  {
  sifre[z]=EEPROM.read(z);
  }

lcd.init();
lcd.clear();
lcd.backlight();

    lcd.setCursor(0, 0); 
    lcd.print("SiFRE/KART/PARMK");  
    lcd.setCursor(3, 1); 
    lcd.print("OKUTUNUZ");  
              
        SPI.begin();               
        mfrc522.PCD_Init();        
        pinMode(yled,OUTPUT);
        pinMode(buzzer,OUTPUT);

        
  

  
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
  
}

void loop() {
//-----------------------------------------------------------------------------------------------------------------
 alinan_parmakizi=getFingerprintIDez();
for(int m=0;m<p;m++)
{  
 if( alinan_parmakizi == parmak_izi_kisi_sayisi[m]) 
 { 
    digitalWrite(yled, HIGH);  // Yeşil ledi yakıyor ve kapıyı açıyor.   
    tone(buzzer,523);       
    lcd.clear();
    lcd.setCursor(2, 0); 
    lcd.print("HOSGELDiNiZ");  
    lcd.setCursor(0, 1); 
    lcd.print("PARMAK iZi DOGRU"); 
    delay(300);
    noTone(buzzer);
    digitalWrite(yled, LOW); 
      
    delay(1200);  
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("SiFRE/KART/PARMK");  
    lcd.setCursor(3, 1); 
    lcd.print("OKUTUNUZ"); 
  }
}
//---------------------------------------------------------------------------------------------------------------
  alinan_tus= tus.getKey(); // tutş takımı kontrol ediliyor basıldımı diye.

 if (alinan_tus)  // herhangibir tuşa basıldıysa bu döngüye gir.
  {
    if(a==0)
    lcd.clear();
    if(alinan_tus=='*') // Eğer yıldıza  basıldıysa bu döngüye giriyor
    {
    sifre_t[a++] = alinan_tus;  
    lcd.setCursor(1, 0); 
    lcd.print("GiRiLEN  SiFRE"); 
    lcd.setCursor((5+a), 1); 
    lcd.print("*");   
    tone(buzzer,523);
    delay(100);
    noTone(buzzer);
    
      sifre_sayac++; // yıldıza basmayı sayıyor
      if(sifre_sayac==3) // 3 defa yıldıza basıldıysa şifre değiştirme döngüsüne giriyor
      {
      sifre_degistir();   // şifreyi değiştirme alt programına gidiyor   
      a=0;              // a değişkeni hem burda hemde şifre değiştirme alt programında kullanıldı dönüşte sıfırlıyoruz karışmasın diye.
    
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("SiFRE/KART/PARMK");  
    lcd.setCursor(3, 1); 
    lcd.print("OKUTUNUZ");  
      }  
    }
    else
    {
    sifre_t[a++] = alinan_tus;

    lcd.setCursor(1, 0); 
    lcd.print("GiRiLEN  SiFRE"); 
    lcd.setCursor((5+a), 1); 
    lcd.print("*");   
    tone(buzzer,523);
    delay(100);
    noTone(buzzer);
    }
  }

  if (a == 4)
  {  
 
    if ((strncmp(sifre_t, sifre, 4) == 0))   ////Girilen şifre(sifre_t) ile kullanıcı şifresini(sifre) kıyaslıyor, doğruysa döngüye giriyor
    {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("HOSGELDiNiZ"); 
      
      digitalWrite(yled, HIGH);  // Yeşil ledi yaıyor ve kapıyıaçıyor.   
      tone(buzzer,523);
      delay(300);
      noTone(buzzer);
      digitalWrite(yled, LOW); 
       delay(1200);   
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("SiFRE/KART/PARMK");  
    lcd.setCursor(3, 1); 
    lcd.print("OKUTUNUZ"); 
      a = 0;
    }

    else
    {
      lcd.clear();
      lcd.setCursor(1, 0); 
      lcd.print("YANLIS SiFRE"); 
      
    
      tone(buzzer,392);
      delay(700);
      noTone(buzzer);
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("SiFRE/KART/PARMK");  
    lcd.setCursor(3, 1); 
    lcd.print("OKUTUNUZ"); 
      a = 0;
    }
  }

//---------------------------------------------------------------------------------------------------------------------------------------------  
if (mfrc522.PICC_IsNewCardPresent()) // Kart okutma döngüsü
        {   
        if (mfrc522.PICC_ReadCardSerial())
        {      
       Serial.println(" Kart bulundu =  ");          
         }
    for ( byte i = 0; i < mfrc522.uid.size; i++)  // Seri ekrana kartın numarasını yazdırıyoruz
        {
          Serial.print(",");
          Serial.print(mfrc522.uid.uidByte[i]);                 
        }
       Serial.println();  // alt satıra geçmek için yazıldı

          for(int i=0 ; i<kartsayisi; i++)
          {
            for(int k=0 ; k<4; k++)
            {
           if (mfrc522.uid.uidByte[k] != kartlar[k+(i*4)])
           izin[k]=false;
           else
           izin[k]=true;
          }
          ei=i;
          for (int k=0;k<4;k++)
          {
            if(izin[k]==true)
            {
             i=kartsayisi;
            onay=true; 
            }
            
            else
            {
            i=ei;
            onay=false;
            k=4;
            }
          }
          }
          
  if(onay==true)
  {
    lcd.clear();
    lcd.setCursor(2, 0); 
    lcd.print("HOSGELDiNiZ"); 
     
  digitalWrite(yled,HIGH);
   tone(buzzer,523);
  delay(300);
  noTone(buzzer);
  digitalWrite(yled,LOW);
   delay(1200);
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("SiFRE/KART/PARMK");  
    lcd.setCursor(3, 1); 
    lcd.print("OKUTUNUZ"); 
  }
  else
  { 
    lcd.clear();
    lcd.setCursor(1, 0); 
    lcd.print("TANIMSIZ KART"); 
   tone(buzzer,392);
  delay(700);
  noTone(buzzer);
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("SiFRE/KART/PARMK");  
    lcd.setCursor(3, 1); 
    lcd.print("OKUTUNUZ"); 
  }   
     mfrc522.PICC_HaltA(); 
    }
   }


//-------------------------------------------------------------------------------------------------------------------------------------------
   void sifre_degistir()  // Şifre değiştirme alt programı
   {
    a=0;
    sifre_sayac=0;
    lcd.clear();
    lcd.setCursor(4, 0); 
    lcd.print("YONETiCi");  
    lcd.setCursor(0, 1); 
    lcd.print("SiFRESi GiRiNiZ!");
    int s=1;
       while (s==1)   // Buradaki işlemler bitene kadar burda döngüde kalıyoruz
       {

       alinan_tus= tus.getKey();

            if (alinan_tus)  
            {
                if(a==0)
                lcd.clear();   
            sifre_t[a++] = alinan_tus;  

            lcd.setCursor(1, 0); 
            lcd.print("GiRiLEN  SiFRE"); 
            lcd.setCursor((5+a), 1); 
            lcd.print("*");   
            tone(buzzer,523);
            delay(100);
            noTone(buzzer);
            }
               if(a==4)
               { 
                
                  if ((strncmp(sifre_t, y_sifre, 4) == 0))   //Girilen şifre(sifre_t) ile yönetici şifresini(y_sifre) kıyaslıyor, doğruysa döngüye giriyor
                  {
                  lcd.clear();
                  lcd.setCursor(3, 0); 
                  lcd.print("YENi SiFRE");  
                  lcd.setCursor(4, 1); 
                  lcd.print("GiRiNiZ!");
                  int k=0;
                  int m=1;
                      while(m==1)
                      {
                       alinan_tus= tus.getKey();
                         if (alinan_tus)  
                         {
                            if(k==0)
                            lcd.clear();   
                         sifre[k] = alinan_tus;
                         EEPROM.write(k,alinan_tus); // Yeni şifreyi eeproma yazıyoruz                          
                          k++;
                         lcd.setCursor(1, 0); 
                         lcd.print("GiRiLEN  SiFRE"); 
                         lcd.setCursor((5+k), 1); 
                         lcd.print(alinan_tus);   
                         tone(buzzer,523);
                         delay(100);
                         noTone(buzzer);
                         }
                            if(k==4)
                            m=0; 
                      }
                      delay(300);    
                      lcd.setCursor(0, 0); 
                      lcd.print("SFRE DEGiSTiRLDi");  
                      delay(2000);
                  }  
                else{
                lcd.clear();
                lcd.setCursor(4, 0); 
                lcd.print("YONETiCi");  
                lcd.setCursor(1, 1); 
                lcd.print("SiFRESi YANLIS!");
                delay(2000);
                }
                                   
           s=0; }
    
    }
    
 }
        
//---------------------------Parmak izi okuma alt programı-----------------------------
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
