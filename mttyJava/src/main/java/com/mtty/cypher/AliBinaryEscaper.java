package com.mtty.cypher;

/**
 * Created by guoze.lin on 16/2/24.
 */
public class AliBinaryEscaper {

    private static final String libraryName = "/libmttyutil.so";

    static {
        try {
            String libPath = System.getProperty("MTTY_LIBPATH");
            System.load(libPath+libraryName);
        }catch(Exception e){
            System.out.println(e.getMessage());
        }
    }

    public static native byte[] unescape(byte[] bytes);
    public static native byte[] escape(byte[] bytes);

    public static void main(String[] args){
        AliBinaryEscaper escaper = new AliBinaryEscaper();
        System.out.println("ok");
        System.exit(0);
    }

}

