/*
 * Copyright 2007 Yusuke Yamamoto
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;

import twitter4j.StallWarning;
import twitter4j.Status;
import twitter4j.StatusDeletionNotice;
import twitter4j.StatusListener;
import twitter4j.TwitterException;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;

/**
 * <p>This is a code example of Twitter4J Streaming API - sample method support.<br>
 * Usage: java twitter4j.examples.PrintSampleStream<br>
 * </p>
 *
 * @author Yusuke Yamamoto - yusuke at mac.com
 */
public final class PrintSampleStream {
	
	public static String lengthFormat(String s, int l) {
		return String.format("%" + l + "s", s).substring(0, l); 
	}
	
    /**
     * Main entry of this application.
     *
     * @param args arguments doesn't take effect with this example
     */
	static int numberOfTweets = 0;
	static int maxLength = 0;
    public static void main(String[] args) throws TwitterException, FileNotFoundException {
//    	String[] argsInput = { "output.txt", "100" };
//    	args = argsInput;
    	if (args.length != 2) {
    		System.err.println("Usage: java PrintSampleStream.java <output> <numberOfTweets>");
    		System.exit(1);
    	}
    	final RandomAccessFile outputFile = new RandomAccessFile(new File(args[0]), "rwd");

    	final int timeLength = 13 + "\0".length(); //"1415927476669".length() == 13;
    	final int screenNameLength = 15 + "\0".length();
    	final int textLength = 140 + "\0".length();
    	final int numberOfFollowersLength = 9 + "\0".length();
    	final int totalTweetLength = timeLength + screenNameLength + textLength + numberOfFollowersLength + "\n".length();
    	final int totalNumberOfTweets = Integer.parseInt(args[1]);
    	long oldestDate = Long.MAX_VALUE;
    	int indexOfOldestDate = 0;
    	for (int i = 0; i < totalNumberOfTweets; ++i) {
    		byte[] bytesRead = new byte[timeLength];
    		try {
    			outputFile.seek(i * totalTweetLength);
    			outputFile.read(bytesRead, 0, timeLength); // string to store, starting store, number of bytes to read
    			long currentDate = Long.parseLong(new String(bytesRead, "UTF-8"));
//    			System.out.println(currentDate);
    			if (currentDate < oldestDate) {
    				indexOfOldestDate = i;
    				oldestDate = indexOfOldestDate;
    			}
			} catch (IOException | NumberFormatException e) {
				break;
			}
    	}
    	numberOfTweets = indexOfOldestDate;
    	try {
    		System.out.println("numberOfTweets = " + numberOfTweets);
			outputFile.seek(indexOfOldestDate * totalTweetLength);
		} catch (IOException e) { 
			e.printStackTrace();
		}
    	
//    	final String beforeSource = "rel=\"nofollow\">";
//    	final String afterSource = "</a>";
    	
    	TwitterStream twitterStream = new TwitterStreamFactory().getInstance();
    	StatusListener listener = new StatusListener() {
    		@Override
    		public void onStatus(Status status) {
    			
//    			String time = lengthFormat("" + status.getTimeStamp(), 13);
//    			String screenName = lengthFormat(status.getUser().getScreenName(), screenNameLength);
//    			String tweetText = lengthFormat(status.getText(), tweetTextLength);
//    			
    			String time = "" + status.getTimeStamp() + '\0';
    			String screenName = status.getUser().getScreenName() + '\0';
    			String tweet = status.getText() + '\0';
    			String numberOfFollowers = "" + status.getUser().getFollowersCount() + '\0';
    			
    			try {
    				outputFile.seek(numberOfTweets * totalTweetLength);
    				outputFile.writeBytes(time);
    				outputFile.seek(numberOfTweets * totalTweetLength + timeLength);
    				outputFile.writeBytes(screenName);
    				outputFile.seek(numberOfTweets * totalTweetLength + timeLength + screenNameLength);
    				outputFile.writeBytes(tweet);
    				outputFile.seek(numberOfTweets * totalTweetLength + timeLength + screenNameLength + textLength);
    				outputFile.writeBytes(numberOfFollowers);
    				outputFile.seek(numberOfTweets * totalTweetLength + timeLength + screenNameLength + textLength + numberOfFollowersLength);
    				outputFile.writeBytes("\n");
//    				outputFile.writeBytes(time + screenName + tweetText + "\n");
				} catch (IOException e) {
					e.printStackTrace();
				}
    			
    			if (++numberOfTweets >= totalNumberOfTweets) {
    				System.out.println("done.");
    				try {
						outputFile.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
    				System.exit(0);
    			}
    		}

    		@Override
    		public void onDeletionNotice(StatusDeletionNotice statusDeletionNotice) {
    			//                System.out.println("Got a status deletion notice id:" + statusDeletionNotice.getStatusId());
    		}

    		@Override
    		public void onTrackLimitationNotice(int numberOfLimitedStatuses) {
    			//                System.out.println("Got track limitation notice:" + numberOfLimitedStatuses);
    		}

    		@Override
    		public void onScrubGeo(long userId, long upToStatusId) {
    			//                System.out.println("Got scrub_geo event userId:" + userId + " upToStatusId:" + upToStatusId);
    		}

    		@Override
    		public void onStallWarning(StallWarning warning) {
    			//                System.out.println("Got stall warning:" + warning);
    		}

    		@Override
    		public void onException(Exception ex) {
    			//                ex.printStackTrace();
    		}
    	};
    	twitterStream.addListener(listener);
    	twitterStream.sample();
    }
}
