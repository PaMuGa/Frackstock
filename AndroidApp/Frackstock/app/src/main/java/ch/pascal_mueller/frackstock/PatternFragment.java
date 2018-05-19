package ch.pascal_mueller.frackstock;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;


/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link DataExchangeInteractionListener} interface
 * to handle interaction events.
 */
public class PatternFragment extends Fragment implements DataExchangeInteractionListener {
    private DataExchangeInteractionListener mListener;

    private ImageButton iBPurpleRain = null;
    private ImageButton iBWhiteFlash = null;
    private ImageButton iBColorFlash = null;
    private ImageButton iBColorful = null;

    public PatternFragment() {
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
        View view =  inflater.inflate(R.layout.fragment_pattern, container, false);

        iBPurpleRain = view.findViewById(R.id.iBPurpleRain);
        iBWhiteFlash = view.findViewById(R.id.iBWhiteFlash);
        iBColorFlash = view.findViewById(R.id.iBColorFlash);
        iBColorful = view.findViewById(R.id.iBColorful);

        iBPurpleRain.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] config_pattern = {0x02, 0x05};
                if(mListener != null) {
                    mListener.onDataSend(config_pattern);
                }
            }
        });

        iBWhiteFlash.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] config_pattern = {0x02, 0x03};
                if(mListener != null) {
                    mListener.onDataSend(config_pattern);
                }
            }
        });

        iBColorFlash.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] config_pattern = {0x02, 0x02};
                if(mListener != null) {
                    mListener.onDataSend(config_pattern);
                }
            }
        });

        iBColorful.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] config_pattern = {0x02, 0x01};
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
    }
}
