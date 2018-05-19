package ch.pascal_mueller.frackstock;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;


/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link DataExchangeInteractionListener} interface
 * to handle interaction events.
 */
public class ConfigurationFragment extends Fragment implements DataExchangeInteractionListener {
    private TextView tvBattery = null;
    private Switch switchMasterMode = null;
    private TextView tvPattern = null;

    private DataExchangeInteractionListener mListener;

    public ConfigurationFragment() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(getActivity() instanceof MainControlActivity)
        {
            ((MainControlActivity) getActivity()).setDataListener(this, 1);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_configuration, container, false);

        tvBattery = view.findViewById(R.id.tVBattery);
        switchMasterMode = view.findViewById(R.id.switchMasterMode);
        tvPattern = view.findViewById(R.id.tVActualPattern);

        switchMasterMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                byte enabled = 0;
                if(b) enabled = 0x01;
                byte [] config_master_on_off = {0x03, enabled};
                if(mListener!=null)
                    mListener.onDataSend(config_master_on_off);
            }
        });

        return view;
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (context instanceof DataExchangeInteractionListener) {
            mListener = (DataExchangeInteractionListener) context;
        } else {
            throw new RuntimeException(context.toString()
                    + " must implement DataExchangeInteractionListener");
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mListener = null;
    }


    @Override
    public void onDataSend(byte[] data) {
        if(data.length > 0)
        {
            switch (data[0])
            {
                case 0x01:
                    tvBattery.setText("Battery: " + data[1] + "%");
                    boolean Master = false;
                    if(data[2] == 1) Master = true;
                    switchMasterMode.setChecked(Master);
                    int pattern = data[3];
                    tvPattern.setText("Pattern: " + PatternNToText(pattern));
                    break;
            }
        }
    }

    private String PatternNToText(int pattern)
    {
        switch (pattern)
        {
            case 0:
                return "Color";
            case 1:
                return "Rainbow";
            case  2:
                return "Flash";
            case 3:
                return "Flashwhite";
            case 4:
                return "Shift";
            case 5:
                return "Purple Rain";
            case 6:
                return "Colorful";
            default:
                return "";
        }
    }
}
