package ch.pascal_mueller.frackstock;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.flask.colorpicker.ColorPickerView;
import com.flask.colorpicker.OnColorChangedListener;


/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link DataExchangeInteractionListener} interface
 * to handle interaction events.
 */
public class ColorFragment extends Fragment implements DataExchangeInteractionListener {

    private ColorPickerView colorPickerView = null;

    private DataExchangeInteractionListener mListener;

    public ColorFragment() {
        // Required empty public constructor
    }


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(getActivity() instanceof MainControlActivity)
        {
            ((MainControlActivity) getActivity()).setDataListener(this,2);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_color, container, false);

        colorPickerView = view.findViewById(R.id.color_picker_view);

        colorPickerView.addOnColorChangedListener(new OnColorChangedListener() {
            @Override
            public void onColorChanged(int i) {
                byte r = (byte)((i >> 16) & 0xFF);
                byte g = (byte)((i >> 8) & 0xFF);
                byte b = (byte)(i & 0xFF);
                byte[] config_pattern = {0x04, r, g, b};
                if(mListener != null) {
                    mListener.onDataSend(config_pattern);
                }
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
                    + " must implement OnFragmentInteractionListener");
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mListener = null;
    }

    @Override
    public void onDataSend(byte[] data) {

    }

    @Override
    public void onResume() {
        super.onResume();
        //selectColorMode();
    }

    public void selectColorMode()
    {
        byte[] config_pattern = {0x02, 0x00};
        if(mListener != null) {
            mListener.onDataSend(config_pattern);
        }
    }
}
