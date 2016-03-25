/**
 * Created by guoze.lin on 16/2/25.
 */
public class Test {

    static int ipStringToInt(String ipString){
        try{
            int result = 0;
            String[] seqs = ipString.split("\\.");
            result |= Integer.parseInt(seqs[3]);
            result |= Integer.parseInt(seqs[2])<<8;
            result |= Integer.parseInt(seqs[1])<<16;
            result |= Integer.parseInt(seqs[0])<<24;
            return result;
        }catch(Exception e){

        }
        return 0;
    }

    static String ipIntToString(int ip){
        return String.format("%d.%d.%d.%d",0xff&(ip>>24),0xff&(ip>>16),0xff&(ip>>8),ip&0xff);
    }

    public static void main(String[] args){
        String ip = "192.168.3.2";
        int ipInt = ipStringToInt(ip);
        System.out.println(ipIntToString(ipInt));
        System.exit(0);
    }
}

