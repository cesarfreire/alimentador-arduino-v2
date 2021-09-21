// --- Bibliotecas Auxiliares ---
#include <LiquidCrystal.h>                              //Biblioteca para o display LCD
#include <DS3231.h>                                     //Biblioteca do RTC
#include <Wire.h>                                       //Biblioteca do Wire para manipular os dados i2c do RTC
#include <Stepper.h>                                    //Biblioteca do motor de passo
#include <avr/wdt.h>                                    //Biblioteca do WatchDog
#include <EEPROM.h>                                     //Biblioteca do EEPROM

// --- Mapeamento de Hardware ---
#define Botoes    A0                                    //Estrada analógica dos botões do shield

// --- Protótipo das Funções Auxiliares ---
void changeMenu();                                      //Função para modificar o menu atual
void dispMenu();                                        //Função para mostrar o menu atual
void lerbotoes();                                       //Função que le o estado dos botoes
void data_hora();                                       //Função do menu1, data e hora
void giraragora();                                      //Função de girar o motor agora
void definirquant();                                    //Função para definir a quantidade a ser despejada
void horario1();                                        //Função de selecionar o período
void horario2();                                        //Função de selecionar o período
void horario3();                                        //Função de selecionar o período


// --- Variáveis Globais ---
int menu = 1;                                           //Variável para selecionar o menu
int set1 = 0, set2 = 0;                                 //Controle das lâmpadas
boolean t_butUp, t_butDown, t_butP, t_butM, t_butS ;    //Flags para armazenar o estado dos botões
boolean f_butUp, f_butDown, f_butP, f_butM, f_butS ;    //Flags para armazenar a função dos botões
const int stepsPerRevolution = 600;                     //Passos por revolução do motor de passo
bool rodar = true;                                      //Boolean para checar se já foi girado o motor em 1 minuto

// --- Variáveis das escolhas dos horários

byte horasval1= 1;
byte horasval2= 1;
byte horasval3= 1;
byte minutosval1, minutosval2, minutosval3 =0;          //Variável com valor inicial dos minutos
byte numvoltas =1;                                      //Variável com valor inicial da quantidade


// --- Mapeamento do motor de passo ---

Stepper passos(stepsPerRevolution, 3, 11, 12, 13);

// --- Hardware do LCD ---
LiquidCrystal disp(8, 9, 4, 5, 6, 7);                   //Pinos utilizados pelo display LCD

// --- Criação dos objetos ---
DS3231 rtc;                                             //Criação do objeto do tipo DS3231
RTCDateTime dataehora;                                  //Criação do objeto do tipo RTCDateTime

// --- Configurações Iniciais ---
void setup()
{
  disp.begin(16,2);                                     //Inicializa LCD 16 x 2
  rtc.begin();                                          //Inicializa o RTC
  pinMode(Botoes, INPUT);                               //Entrada analógica A0 para leitura dos botões
  
  t_butUp   = 0x00;                                     //limpa flag do botão Up
  t_butDown = 0x00;                                     //limpa flag do botão Down
  t_butP    = 0x00;                                     //limpa flag do botão P
  t_butM    = 0x00;                                     //limpa flag do botão M

// --- Definir a hora, dia da semana e o ano do RTC ---
  //rtc.setDOW(WEDNESDAY);                              //Setar o dia da semana em INGLES
  //rtc.setTime(19, 22, 0);                             //Seta o horário. Ex.: para 12:00:00 (24hr format)
  //rtc.setDate(26, 10, 2017);                          //Seta a data. Ex.: 20 de Agosto de 2017

//rtc.setDateTime(__DATE__, __TIME__);

  passos.setSpeed(60);                                  //Define velocidade do motor de passo

  wdt_enable(WDTO_4S);                                  //Escolha do tempo, caso ultrapasse o tempo sem o reset do watchdog irá reiniciar o programa
  delay(10);

  horasval1= EEPROM.read(1);                            //Le os valores salvos na memória EEPROM
  horasval2= EEPROM.read(3);
  horasval3= EEPROM.read(5);
  minutosval1= EEPROM.read(2);
  minutosval2= EEPROM.read(4);
  minutosval3= EEPROM.read(6);           
  numvoltas= EEPROM.read(0);
  

} //end setup


// --- Loop Infinito ---
void loop()                                                        //A estrutura de looping fica realizando as funções
{  
  dataehora = rtc.getDateTime();                                   //Obtem a data e a hora e 'escreve' na variável
  lerbotoes();                                                     //Verifica o estado dos botões
  changeMenu();													   //Muda a tela conforme a selecionada
  dispMenu();                                                      //Exibe no display o menu selecionado
  horaprogramada();												   //Verifica se está no horário para girar o motor
  wdt_reset();                                                     //Reseta o WATCHDOG
} //end loop

// --- Desenvolvimento das Funções Auxiliares ---
void lerbotoes()
{
  int valorbotao = analogRead(Botoes);
  if(valorbotao > 340 && valorbotao < 350) t_butUp = 1;             //Botão CIMA pressionado? Seta flag
  if(valorbotao > 1020 && t_butUp)                                  //Botão CIMA solto e flag setada
   {                                                                //Sim...
      t_butUp = 0;                                                  //Seta a flag em 0, não apertado
      f_butUp = 1;                                                  //Ativa função Botao CIMA
   }
  
  else if(valorbotao > 140 && valorbotao < 150) t_butDown = 1;      //Botão BAIXO pressionado? Seta flag
  if(valorbotao > 1020 && t_butDown)                                //Botão BAIXO solto e flag setada?
  {                                                                 //Se for sim...
    t_butDown = 0;                                                  //Limpa flag
    f_butDown = 1;                                                  //Ativa função Botao BAIXO
  }
  
  else if(valorbotao > 500 && valorbotao < 520)   t_butP    = 0x01; //Botão P pressionado? Seta flag
   if(valorbotao > 1020 && t_butP)                                  //Botão P solto e flag setada?                         // BOTÃO LADO DIREITO
   {                                                                //Se for sim...
     t_butP = 0;                                                    //Limpa flag
     f_butP = 1;                                                    //Ativa função Botao p
   }
   
  else if(valorbotao < 5) t_butM = 0x01;                            //Botão M pressionado? Seta flag
   if(valorbotao > 1020 && t_butM)                                  //Botão M solto e flag setada?                         // BOTÃO LADO ESQUERDO
   {                                                                //Se for sim...
      t_butM = 0;                                                   //Limpa flag
      f_butM = 1;                                                   //Ativa função Botao M  
   }
  else if(valorbotao > 740 && valorbotao < 750) t_butS = 0x01;      //Botão M pressionado? Seta flag
   if(valorbotao > 1020 && t_butS)                                  //Botão M solto e flag setada?                         // BOTÃO SELECT
   {                                                                //Se for sim...
      t_butS = 0;                                                   //Limpa flag
      f_butS = 1;                                                   //Ativa função Botao M  
   }
  
}// end lerbotoes


void changeMenu()                                                   //Função para modificar o menu atual
{
  
  if(f_butUp){                                                      //Se a função do botão CIMA foi ATIVA
      disp.clear();                                                 //Limpa display
      menu++;                                                       //Incrementa o menu
      
      if(menu > 0x06) menu = 0x01;                                  //Se menu maior que 6, volta a ser 1
   f_butUp =0;                                                      //Limpa função CIMA
   } //end butUp
   
   
  
  if(f_butDown){                                                    //Se a função do botão BAIXO foi ATIVA                                                                 //Sim...
     disp.clear();                                                  //Limpa display
      menu--;                                                       //Decrementa o menu
      
      if(menu < 0x01) menu = 0x06;                                  //Se menu menor que 1, volta a ser 6
      f_butDown =0;                                                 //Limpa função BAIXO
   } //end butDown   

} //end changeMenu

void dispMenu()                                                     //Mostra o menu atual
{
    switch(menu)                                                    //Controle da variável menu
    {
       case 0x01:                                                   //Caso 1
             data_hora();                                           //Chama a função da marca e da hora
            
             break;                                                 //break
       case 0x02:                                                   //Caso 4
             definirquant();                                        //Chama função para o definir a quantidade
             
             break;                                                 //break
       case 0x03:                                                   //Caso 4
             horario1();                                            //Chama função para o controle dos minutos
             
             break;                                                 //break
       case 0x04:                                                   //Caso 4
             horario2();                                            //Chama função para o controle dos minutos
             
             break;                                                 //break
       case 0x05:                                                   //Caso 4
             horario3();                                            //Chama função para o controle dos minutos
             
             break;                                                 //break
       case 0x06:
             giraragora();                                          //Chama a função para girar o motor agora

             break;
        } //end switch menu
} //end dispMenu

void data_hora()                                        //Menu de Data e Hora
{
    
   disp.setCursor(4,0);                                 //Posiciona cursor na coluna 1, linha 1
   disp.print("AutoPets");                              //Imprime a marca
   disp.setCursor(1,1);                                 //Posiciona cursor na coluna 1, linha 2
   disp.print(dataehora.hour);                          //Informa as horas atraves do RTC
   disp.print(":");
   disp.print(dataehora.minute);
   disp.print("h");
   disp.setCursor(9,1);
   disp.print(rtc.readTemperature());                   //Exibe a temperatura
   disp.print("C");

   if (f_butP)                                          //Se a função do botão P (DIREITA) for acionada
   {
    disp.setCursor(0,1);                                //Seta o cursor na segunda linha
    disp.print("                ");                     //"Apaga" a linha inteira
    disp.setCursor(3,1);                                //Seta o cursor na terceira coluna, segunda linha
    disp.print(dataehora.day);                          //Imprime o dia
    disp.print("-");
    disp.print(dataehora.month);                        //Imprimindo o Mês
    disp.print("-");
    disp.print(dataehora.year);                         //Imprime o ano
    delay(5000);                                        //Delay de 5 segundos exibindo a data
    disp.setCursor(0,1);                                //Seta o cursor na coluna 1, linha 2
    disp.print("                ");                     //"Apaga" a linha inteira
    f_butP = 0;                                         //Seta a função do botão P (DIREITA) para 0
   }
} //end data_hora

void horario1()                                         //Função para definir o intervalo
{
   disp.clear();
   disp.setCursor(0,0);                                 //Posiciona cursor na coluna 1, linha 1
   disp.print("Selec. horario 1");                      //Imprime título da tela
   disp.setCursor(1,1);                                 //Posiciona cursor na coluna 1, linha 2
   disp.print(horasval1);                               //Exibe o valor das horas
   disp.print(" Hrs");                                  //Exibe unidade de medida
   disp.setCursor(9,1);                                 //Seta o cursor na coluna 9, linha 2
   disp.print(minutosval1);                             //Exibe o valor dos minutos
   disp.print(" min");                                  //Unidade de medida após a variável
  
  if(f_butP)                                            //Se a função do botão P (DIREITA) está em 1
  {
    minutosval1=minutosval1+10;                         //Incrementa valor dos minutos em 10
    f_butP =0;                                          //Zera a função
  }
  if(f_butM)                                            //Se a função do botão M (ESQUERDA) está em 1
  {
    horasval1=horasval1+1;                              //Incrementa o valor do das horas
    f_butM =0;                                          //Zera a função
  }
  if(horasval1>23) horasval1=1;                         //Se for maior que 23 horas, retorna ao valor 1
  if(horasval1<1) horasval1=23;                         //Se for menor que 1 hora, retorna ao valor de 23 horas
  if(minutosval1>60) minutosval1 = 0;                   //Se for maior que 60 minutos, retorna a 0
  if(minutosval1<0) minutosval1 = 59;                   //Se for menor que 0 minutos, retorna a 60
  if(minutosval1==60) minutosval1 = 59;

  if (f_butS)                                           //Se a função do botão 'SELECT' for 1
  {
    disp.clear();
    disp.setCursor(3,1);
    EEPROM.write(1, horasval1);                         //Salva o valor da variável horasval1 na EEPROM, endereço 1
    delay(10);
    EEPROM.write(2, minutosval1);                       //Salva o valor da variável minutosval1 na EEPROM, endereço 2
    delay(10);
    disp.print("Habilitado!");
    delay(3000);
    disp.clear();
    f_butS =0;
  }
}

void horario2()                                         //Função para definir o intervalo
{
   disp.clear();
   disp.setCursor(0,0);                                 //Posiciona cursor na coluna 1, linha 1
   disp.print("Selec. horario 2");                      //Imprime título da tela
   disp.setCursor(1,1);                                 //Posiciona cursor na coluna 1, linha 2
   disp.print(horasval2);                               //Exibe o valor das horas
   disp.print(" Hrs");                                  //Exibe unidade de medida
   disp.setCursor(9,1);                                 //Seta o cursor na coluna 9, linha 2
   disp.print(minutosval2);                             //Exibe o valor dos minutos
   disp.print(" min");                                  //Unidade de medida após a variável
  
  if(f_butP)                                            //Se a função do botão P (DIREITA) está em 1
  {
    minutosval2=minutosval2+10;                         //Incrementa valor dos minutos em 10
    f_butP =0;                                          //Zera a função
  }
  if(f_butM)                                            //Se a função do botão M (ESQUERDA) está em 1
  {
    horasval2=horasval2+1;                              //Incrementa o valor do das horas
    f_butM =0;                                          //Zera a função
  }
  if(horasval2>23) horasval2=1;                         //Se for maior que 23 horas, retorna ao valor 1
  if(horasval2<1) horasval2=23;                         //Se for menor que 1 hora, retorna ao valor de 23 horas
  if(minutosval2>60) minutosval2 = 0;                   //Se for maior que 60 minutos, retorna a 0
  if(minutosval2<0) minutosval2 = 59;                   //Se for menor que 0 minutos, retorna a 59
  if(minutosval2==60) minutosval2 = 59;                 //Se for igual a 60, então fica 59


  if (f_butS)                                           //Se a função do botão 'SELECT' for 1
  {
    disp.clear();
    disp.setCursor(3,1);
    EEPROM.write(3, horasval2);                         //Salva o valor da variável horasval2 na EEPROM, endereço 3
    delay(10);
    EEPROM.write(4, minutosval2);                       //Salva o valor da variável minutosval2 na EEPROM, endereço 4
    delay(10);
    disp.print("Habilitado!");
    delay(3000);
    disp.clear();
    f_butS =0;
  }
}

void horario3()                                         //Função para definir o intervalo
{
   disp.clear();
   disp.setCursor(0,0);                                 //Posiciona cursor na coluna 1, linha 1
   disp.print("Selec. horario 3");                      //Imprime título da tela
   disp.setCursor(1,1);                                 //Posiciona cursor na coluna 1, linha 2
   disp.print(horasval3);                               //Exibe o valor das horas
   disp.print(" Hrs");                                  //Exibe unidade de medida
   disp.setCursor(9,1);                                 //Seta o cursor na coluna 9, linha 2
   disp.print(minutosval3);                             //Exibe o valor dos minutos
   disp.print(" min");                                  //Unidade de medida após a variável
  
  if(f_butP)                                            //Se a função do botão P (DIREITA) está em 1
  {
    minutosval3=minutosval3+10;                         //Incrementa valor dos minutos em 10
    f_butP =0;                                          //Zera a função
  }
  if(f_butM)                                            //Se a função do botão M (ESQUERDA) está em 1
  {
    horasval3=horasval3+1;                              //Incrementa o valor do das horas
    f_butM =0;                                          //Zera a função
  }
  if(horasval3>23) horasval3=1;                         //Se for maior que 23 horas, retorna ao valor 1
  if(horasval3<1) horasval3=23;                         //Se for menor que 1 hora, retorna ao valor de 23 horas
  if(minutosval3<0) minutosval3 = 59;                   //Se for menor que 0 minutos, retorna a 59
  if(minutosval3==60) minutosval3 = 59;                 //Se for igual a 60, então fica 59

  if (f_butS)                                           //Se a funlção do botão 'SELECT' for 1
  {
    disp.clear();
    disp.setCursor(3,1);
    EEPROM.write(5, horasval3);                         //Salva o valor da variável horasval3 na EEPROM, endereço 5
    delay(10);
    EEPROM.write(6, minutosval3);                       //Salva o valor da variável minutosval3 na EEPROM, endereço 6
    delay(10);
    disp.print("Habilitado!");
    delay(3000);
    disp.clear();
    f_butS =0;
  }
}


void definirquant()                                     //Menu para definir a quantidade de ração
{
   disp.clear();
   disp.setCursor(0,0);                                 //Posiciona cursor na coluna 1, linha 1
   disp.print("Selec. o numero");                       //Imprime o título
   disp.setCursor(0,1);                                 //Posiciona cursor na coluna 0, linha 2
   disp.print(" de voltas = "); 
   disp.print(numvoltas);                               //Imprime o valor da variável da quantidade
  
  if(f_butP)                                            //Se a função do botão P (DIREITA) está ativo
  {
    numvoltas=numvoltas+1;                              //incrementa valor da quantidade em 100 gramas
    f_butP =0;                                          //Zera a função
  }
  if(f_butM)                                            //Se a função do botão M (ESQUERDA) está ativo
  {
    numvoltas=numvoltas-1;                              //decrementa valor da quantidade em 100 gramas
    f_butM =0;                                          //Zera a função
  }
  if(numvoltas>5) numvoltas=1;                          //Se o número de voltas for maior que 5, retorna ao valor 1
  if(numvoltas<1) numvoltas=5;                          //Se o número de voltas for menor que 1, retorna o valor 5

  if (f_butS)                                           //Se o botão 'SELECT' for pressionado
  {
    disp.clear();
    disp.setCursor(0,1);
    EEPROM.write(0, numvoltas);                         //Salva o valor na memória EEPROM
    delay(10);
    disp.print(" Numero salvo!");                       //Imprime a mensagem
    delay(3000);
    disp.clear();
    f_butS =0;                                          //Seta a função do botão 'SELECT' em zero
  }
}

void giraragora()                                       //Menu para girar o motor e dispensar ração
{
  disp.setCursor(0,0);
  disp.print("Alimentar agora");
  disp.setCursor(0,1);
  disp.print("Aperte p/ girar");
  if (f_butP)                                           //Se a função do botão P (DIREITA) estiver em 1
  {
    disp.clear();
    disp.setCursor(3,0);
    disp.print("Aguarde...");
    disp.setCursor(1,1);
    disp.print("Alimentando...");
    rotacionar();									    //Chama a função de rotacionar o motor e dispensar a ração
    disp.clear();
    disp.setCursor(3,1);
    disp.print("Concluido!");                           //Imprime a mensagem de conclusão do processo
    delay(3000);
    f_butP = 0;
  }


}


void horaprogramada() {
  if ((horasval1 == dataehora.hour) && (minutosval1 == dataehora.minute)){        //Se os valores de horas e minutos escolhidos forem iguais ao horário atual
  
	if(rodar==true){															  //Se a variável rodar for TRUE
		rotacionar();                                                             //Chama a função de rotacionar o motor
		rodar=false;															  //Seta a variável para FALSE
	}
  }

  if ((horasval2 == dataehora.hour) && (minutosval2 == dataehora.minute)){        //Se os valores de horas e minutos escolhidos forem iguais ao horário atual
  
	if(rodar==true){															  //Se a variável rodar for TRUE
		rotacionar();                                                             //Chama a função de rotacionar o motor
		rodar=false;															  //Seta a variável para FALSE
	}
  }

  if ((horasval3 == dataehora.hour) && (minutosval3 == dataehora.minute)){        //Se os valores de horas e minutos escolhidos forem iguais ao horário atual
  
	if(rodar==true){															  //Se a variável rodar for TRUE
		rotacionar();                                                             //Chama a função de rotacionar o motor
		rodar=false;															  //Seta a variável para FALSE
	}
  } else {
	  rodar=true;                                                                 //Se a hora é diferente da escolhida, seta a variável como true
  }
}

void rotacionar() {                                                               //Função de rotacionar o motor
  for (int i = 0; i < numvoltas; i++){                                            //Laço FOR para girar o motor de acordo com a escolha do usuário na variável
 passos.step(-200);																  //Rotaciona o motor
 wdt_reset();                                                                     //Faz o reset do WATCHGOD
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-200);
 wdt_reset();
 passos.step(-48);
 wdt_reset();
  }
}

