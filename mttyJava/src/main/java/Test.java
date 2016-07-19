import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

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

    public static String getDate(Long d, String p) {
        long t = 1000L;
        SimpleDateFormat sdf = new SimpleDateFormat(p);
        sdf.setTimeZone(TimeZone.getTimeZone("GMT"));
        Date date = new Date(d * t);
        return sdf.format(date);
    }

    public static void main(String[] args){
        System.out.println(getDate(1467876463L,"YYYY-MM-dd HH"));
        System.exit(0);
    }
}

