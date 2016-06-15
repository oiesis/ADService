package com.mtty.log;

import com.alibaba.fastjson.JSON;
import com.aliyun.openservices.ons.api.*;

import java.io.IOException;
import java.util.Properties;
import com.mtty.cypher.*;
import com.mtty.protocol.LogItem;
import org.apache.avro.Schema;
import org.apache.avro.file.DataFileReader;
import org.apache.avro.io.BinaryDecoder;
import org.apache.avro.io.DatumReader;
import org.apache.avro.io.Decoder;
import org.apache.avro.io.DecoderFactory;
import org.apache.avro.specific.SpecificDatumReader;

/**
 * Created by guoze.lin on 16/2/25.
 */
public class AliyunLogReader {

    private static final String ConsumerId = "CID_mtty001";
    private static final String PublishTopics = "adlog";
    private static final String AccessKey =  "5jaQzkjjARFVFUrE";
    private static final String SecretKey = "SbFRrY6y1cnSKcdC0QpK1Vkv0QMmTw";

    private static Consumer consumer;

    static{
        Properties properties = new Properties();
        properties.put(PropertyKeyConst.ConsumerId, ConsumerId);
        properties.put(PropertyKeyConst.AccessKey, AccessKey);
        properties.put(PropertyKeyConst.SecretKey, SecretKey);
        try {
            consumer = ONSFactory.createConsumer(properties);
            AliyunLogMessageListener msgListener = new AliyunLogMessageListener();
            msgListener.registerTask(new DefaultMessageTask());
            consumer.subscribe(PublishTopics,"*",msgListener);
        }catch(Exception e){
            System.out.println(e.getMessage());
        }

    }

    public Consumer getConsumer(){
        return consumer;
    }

    public static interface AbstractMessageTask{
        public void doTask(byte[] data);
    }

    public static class DefaultMessageTask implements AbstractMessageTask{

        @Override
        public void doTask(byte[] data) {
            try{
                DatumReader<LogItem> dataReader = new SpecificDatumReader<LogItem>(LogItem.class);
                BinaryDecoder decoder = null;
                decoder = DecoderFactory.get().binaryDecoder(data,decoder);
                LogItem logItem = null;
                logItem = dataReader.read(logItem,decoder);
                String jsonString = JSON.toJSONString(logItem);
                System.out.println(jsonString);
            }catch(Exception e){
                System.out.println(e.getMessage());
            }
        }
    }

    public static class AliyunLogMessageListener implements MessageListener{

        private AbstractMessageTask worker;

        public void registerTask(AbstractMessageTask w){
            worker = w;
        }

        @Override
        public Action consume(Message message, ConsumeContext consumeContext) {
            byte[] data = message.getBody();
            byte[] unescapeData = AliBinaryEscaper.unescape(data);
            worker.doTask(unescapeData);
            return Action.CommitMessage;
        }
    }

    public static void main(String[] args){
        try{
            AliyunLogReader reader = new AliyunLogReader();
            Consumer consumer = reader.getConsumer();
            consumer.start();
        }catch(Exception e){
            System.out.println(e.getMessage());
        }
        System.exit(0);
    }

}
