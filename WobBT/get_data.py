from binance.client import Client
from config import BINANCE,COIN_TARGET, COIN_REFER
import csv,datetime,os
import sys

def get_Date_Data(fromdate,todate,timeframe,target,reGet):
    client = Client(BINANCE.get("key"),BINANCE.get("secret"))
    path = "data/"+target+"-"+COIN_REFER+"_"+timeframe+"_"+str(fromdate)+"="+str(todate)+".csv"
    if(os.path.exists(path) and not reGet):
        print("Data already exist: " + path)
        return path

    csvfile = open(path, 'w', newline='') 
    candlestick_writer = csv.writer(csvfile, delimiter=',')
    
    print("Getting Data of: " + path)
    
    candlesticks = client.get_historical_klines(target + COIN_REFER, timeframe, str(fromdate), str(todate))
    
    for candlestick in  candlesticks:
        candlestick[0] = candlestick[0] / 1000
        timestamp = candlestick[0]
        dt_object = datetime.datetime.fromtimestamp(timestamp)
        candlestick[0] = str(dt_object)
        candlestick_writer.writerow(candlestick)
    
    csvfile.close()
    return path


def initDataDate(fromdate,todate,timeframe,target=COIN_TARGET,refresh=False):
    ### Get Data ###
    path = get_Date_Data(fromdate,todate,timeframe,target,refresh)
    ### Load Data ###
    return path

def initData(traindays,testdays,timeframe,target=COIN_TARGET,refresh=False):
    ### Choose Time period of Backtest ###
    today    = datetime.date.today() #- datetime.timedelta(days=4)
    today    = today - datetime.timedelta(days=testdays)
    fromdate = today - datetime.timedelta(days=traindays)
    todate   = today + datetime.timedelta(days=1)
    ### Get Data ###
    path = initDataDate(fromdate,todate,timeframe,target,refresh=refresh)
    ### Load Data ###
    return path


def StdDateInit(tf):
    fromdate = datetime.datetime.strptime('2021-01-19', '%Y-%m-%d')
    fromdate = fromdate.date()
    todate = datetime.date.today() + datetime.timedelta(days=1)
    return initDataDate(fromdate,todate,Client.KLINE_INTERVAL_15MINUTE,COIN_TARGET,tf)

def StartDateInit(tf):
    fromdate = datetime.datetime.strptime('2022-09-01', '%Y-%m-%d')
    fromdate = fromdate.date()
    todate = datetime.date.today() + datetime.timedelta(days=1)
    return initDataDate(fromdate,todate,Client.KLINE_INTERVAL_15MINUTE,COIN_TARGET,tf)

def DateInit(date):
    fromdate = datetime.datetime.strptime(date, '%Y-%m-%d')
    fromdate = fromdate.date()
    todate = datetime.date.today() + datetime.timedelta(days=1)
    return initDataDate(fromdate,todate,Client.KLINE_INTERVAL_15MINUTE,COIN_TARGET,True)


val_list =list()

if __name__ == '__main__':
    reget = True
    Delay = 0
    Dayz = 132


    if(len(sys.argv) > 1):
        print("BackTest Date: " + str(sys.argv[1]))
        date = str(sys.argv[1])
        DateInit(date)
    else:
        StdDateInit(reget) 


    #initData(Dayz,Delay,Client.KLINE_INTERVAL_15MINUTE,COIN_TARGET,reget)
    #StartDateInit(True) #Standart Date to today test
    #Standart Date to today test
    print("Finised Getting Data...")
