package com.example.camcontrol;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.clj.fastble.BleManager;
import com.clj.fastble.callback.BleGattCallback;
import com.clj.fastble.callback.BleScanCallback;
import com.clj.fastble.callback.BleWriteCallback;
import com.clj.fastble.data.BleDevice;
import com.clj.fastble.exception.BleException;
import com.iflytek.cloud.RecognizerResult;
import com.iflytek.cloud.SpeechConstant;
import com.iflytek.cloud.SpeechError;
import com.iflytek.cloud.SpeechUtility;
import com.iflytek.cloud.ui.RecognizerDialog;
import com.iflytek.cloud.ui.RecognizerDialogListener;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONTokener;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;


public class MainActivity extends AppCompatActivity {

    private TextView txt_command;
    private String[] commandList;

    /*蓝牙部分*/
    private final String arduinoMac = "32:0F:B2:8B:3F:3A";//目标MAC地址
    private BleDevice device;
    private boolean isDeviceFound;//是否找到设备标记
    private String service;
    private String characteristic;

    @RequiresApi(api = Build.VERSION_CODES.S)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        /*初始化布局元素*/
        initLayout();
        /*动态获取权限*/
        requestPermission();
        /*初始化语音识别api*/
        SpeechUtility.createUtility(this, SpeechConstant.APPID + "=adb783c3");
        /*初始化命令内容*/
        commandList = new String[]{"无效命令", "开始跟踪", "停止跟踪"};
        /*初始化蓝牙设置*/
        isDeviceFound = false;
        BleManager.getInstance().enableBluetooth();
        BleManager.getInstance().init(getApplication());
        BleManager.getInstance()
                .enableLog(true)
                .setReConnectCount(1, 5000)
                .setSplitWriteNum(20)
                .setConnectOverTime(10000)
                .setOperateTimeout(5000);
    }

    /**
     * 连接蓝牙设备
     */
    private void connect(){
        BleManager.getInstance().connect(arduinoMac, new BleGattCallback() {
            @Override
            public void onStartConnect() {
                // 开始连接
            }

            @Override
            public void onConnectFail(BleDevice bleDevice, BleException exception) {
                // 连接失败
                String error = "连接失败";
                Toast.makeText(MainActivity.this, error, Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onConnectSuccess(BleDevice bleDevice, BluetoothGatt gatt, int status) {
                // 连接成功，BleDevice即为所连接的BLE设备
                String success = "连接成功";
                Toast.makeText(MainActivity.this, success, Toast.LENGTH_SHORT).show();
                //获取写入的service和characteristic
                List<BluetoothGattService> serviceList = gatt.getServices();
                UUID uuid_service = serviceList.get(2).getUuid();
                service = uuid_service.toString();
                List<BluetoothGattCharacteristic> characteristicList= serviceList.get(2).getCharacteristics();
                UUID uuid_characteristic = characteristicList.get(0).getUuid();
                characteristic = uuid_characteristic.toString();
            }

            @Override
            public void onDisConnected(boolean isActiveDisConnected, BleDevice bleDevice, BluetoothGatt gatt, int status) {
                // 连接中断，isActiveDisConnected表示是否是主动调用了断开连接方法
                String error = "连接失败";
                Toast.makeText(MainActivity.this, error, Toast.LENGTH_SHORT).show();
            }
        });
    }

    /**
     * 扫描并连接蓝牙设备
     */
    private void scanAndConnect() {
        BleManager.getInstance().scan(new BleScanCallback() {
            @Override
            public void onScanStarted(boolean success) {}

            @Override
            public void onLeScan(BleDevice bleDevice) {
                super.onLeScan(bleDevice);
            }

            @Override
            public void onScanning(BleDevice bleDevice) {
                if (bleDevice.getMac().equals(arduinoMac)){
                    device = bleDevice;
                    isDeviceFound = true;
                    connect();
                }
            }

            @Override
            public void onScanFinished(List<BleDevice> scanResultList) {
                if (!isDeviceFound){
                    String not_found = "未找到设备";
                    Toast.makeText(MainActivity.this, not_found, Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    void write(BleDevice bleDevice, String uuid_service, String uuid_characteristic_read, byte[] data){
        BleManager.getInstance().write(bleDevice, uuid_service, uuid_characteristic_read, data,
                new BleWriteCallback() {
                    @Override
                    public void onWriteSuccess(int current, int total, byte[] justWrite) {
                        // 发送数据到设备成功
                        String success = "信号发送成功";
                        Toast.makeText(MainActivity.this, success, Toast.LENGTH_SHORT).show();
                    }

                    @Override
                    public void onWriteFailure(BleException exception) {
                        // 发送数据到设备失败
                        String error = "信号发送失败";
                        Toast.makeText(MainActivity.this, error, Toast.LENGTH_SHORT).show();
                    }
                });
    }

    /**
     * 写入启动命令
     */
    void writeStart(){
        byte[] data = {1};
        write(device, service, characteristic, data);
    }

    /**
     * 写入停止命令
     */
    void writeStop(){
        byte[] data = {0};
        write(device, service, characteristic, data);
    }

    /**
     * 获取动态权限
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    private void requestPermission() {
        if (Build.VERSION.SDK_INT >= 23) {
            ArrayList<String> permissionsList = new ArrayList<>();
            String[] permissions = {
                    Manifest.permission.RECORD_AUDIO,
                    Manifest.permission.INTERNET,
                    Manifest.permission.ACCESS_WIFI_STATE,
                    Manifest.permission.CHANGE_NETWORK_STATE,
                    Manifest.permission.READ_PHONE_STATE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.BLUETOOTH,
                    Manifest.permission.BLUETOOTH_ADMIN,
                    Manifest.permission.BLUETOOTH_CONNECT,
                    Manifest.permission.ACCESS_COARSE_LOCATION,
                    Manifest.permission.ACCESS_FINE_LOCATION
            };

            for (String perm : permissions) {
                if (PackageManager.PERMISSION_GRANTED != checkSelfPermission(perm)) {
                    permissionsList.add(perm);
                    // 进入到这里代表没有权限.
                }
            }
            if (!permissionsList.isEmpty()) {
                String[] strings = new String[permissionsList.size()];
                requestPermissions(permissionsList.toArray(strings), 0);
            }
        }
    }

    /**
     * 初始化布局元素设置
     */
    private void initLayout() {

        //监听按钮初始化
        Button btn_listen = findViewById(R.id.btn_listen);
        txt_command = findViewById(R.id.txt_voice_result);
        btn_listen.setOnClickListener(view -> listen());
        //连接按钮初始化
        Button btn_connect = findViewById(R.id.btn_connect);
        btn_connect.setOnClickListener(view -> scanAndConnect());
        //测试按钮初始化
        Button btn_test = findViewById(R.id.btn_test);
        btn_test.setOnClickListener(view -> {

        });
        //测试用开始跟踪按钮
        Button btn_start = findViewById(R.id.btn_start);
        btn_start.setOnClickListener(view -> writeStart());
        //测试用停止跟踪按钮
        Button btn_stop = findViewById(R.id.btn_stop);
        btn_stop.setOnClickListener(view -> writeStop());
    }

    /**
     * 开启语音识别
     */
    public void listen(){
        RecognizerDialog dialog = new RecognizerDialog(this, null);
        dialog.setParameter(SpeechConstant.LANGUAGE, "zh_cn");
        dialog.setParameter(SpeechConstant.ACCENT, "mandarin");

        dialog.setListener(new RecognizerDialogListener() {
            @Override
            public void onResult(RecognizerResult recognizerResult, boolean b) {
                String original_command = parseIatResult(recognizerResult.getResultString());
                Command command = parseCommand(original_command);
                if (command == Command.start){
                    writeStart();//写入启动命令
                }
                else if (command == Command.stop){
                    writeStop();//写入停止命令
                }
            }

            @Override
            public void onError(SpeechError speechError) {}
        });
        dialog.show();
        //Toast.makeText(this, "请开始说话",Toast.LENGTH_LONG).show();
    }

    /**
     *解析语音指令
     */
    private Command parseCommand(String text){
        if (text.contains(this.commandList[1])){
            this.txt_command.setText(this.commandList[1]);
            return Command.start;
        }
        else if (text.contains(this.commandList[2])){
            this.txt_command.setText(this.commandList[2]);
            return Command.stop;
        }
        else{
            this.txt_command.setText(this.commandList[0]);
            return Command.error;
        }
    }

    /**
     * 解析语音结果json
     */
    private static String parseIatResult(String json) {
        StringBuilder ret = new StringBuilder();
        try {
            JSONTokener tokener = new JSONTokener(json);
            JSONObject joResult = new JSONObject(tokener);
            JSONArray words = joResult.getJSONArray("ws");
            for (int i = 0; i < words.length(); i++) {
                // 转写结果词，默认使用第一个结果
                JSONArray items = words.getJSONObject(i).getJSONArray("cw");
                JSONObject obj = items.getJSONObject(0);
                ret.append(obj.getString("w"));
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        return ret.toString();
    }

}
